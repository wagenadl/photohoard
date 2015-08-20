// Scanner.cpp

#include "Scanner.h"
#include "PDebug.h"
#include <QDir>
#include <QFileInfoList>
#include <system_error>
#include <QDateTime>
#include "Exif.h"
#include "NoResult.h"
#include "Tags.h"

Scanner::Scanner(PhotoDB *db0): db0(db0) {
  setObjectName("Scanner");
  db.clone(*db0);
  QSqlQuery q = db.constQuery("select extension, filetype from extensions");
  while (q.next()) 
    exts[q.value(0).toString()] = q.value(1).toInt();
}

Scanner::~Scanner() {
  stopAndWait();
  db.close();
}

void Scanner::addTree(QString path, QString defaultCollection,
                      QStringList excluded) {
  // This is called from outside of thread!
  foreach (QString e, excluded)
    excludeTree(e);
  
  if (!db0->findFolder(path)) {
    QDir parent(path);
    parent.cdUp();
    QString leaf = parent.relativeFilePath(path);
    QString parentPath = parent.path();
    Transaction t(db0);
    quint64 parentid = db0->defaultQuery("select id from folders"
                                         " where pathname==:a", parentPath, 0)
      .toULongLong();
    quint64 id = addFolder(db0, parentid, path, leaf);
    if (!defaultCollection.isEmpty()) {
      Tags tags(db0);
      int t = tags.ensureCollection(defaultCollection);
      db0->query("insert into defaulttags(folder, tag) values(:a,:b)",
                 id, t);
    }
    t.commit();
  }

  rescan(path);
}

void Scanner::rescanAll() {
  // This is called from outside of thread!
  QSqlQuery q = db0->query("select pathname from folders"
                           " where parentfolder is null");
  QStringList roots;
  while (q.next())
    roots << q.value(0).toString();
  q.finish();

  for (QString r: roots)
    rescan(r);
}

void Scanner::rescan(QString path) {
  // This is called from outside of thread!
  pDebug() << "rescan" << path;
  Untransaction t(db0);
  db0->query("insert into folderstoscan select id from folders"
             " where pathname==:a", path);
  pDebug() << "rescan" << path << "inserted";
  
  QMutexLocker l(&mutex);
  waiter.wakeOne();
}

quint64 Scanner::addPhoto(quint64 parentid, QString leaf) {
  // This is called from inside of thread!
  // Insert into photo table
  int idx = leaf.lastIndexOf(".");
  QString ext = leaf.mid(idx+1).toLower();

  QSqlQuery q
    = db.query("insert into photos(folder, filename, filetype) "
	       " values (:a,:b,:c)",
	       parentid, leaf, exts.contains(ext) ? exts[ext]: QVariant());
  quint64 photo = q.lastInsertId().toULongLong();

  // Create first version - this is preliminary code
  q = db.query("insert into versions(photo) values(:a)", photo);
  quint64 vsn = q.lastInsertId().toULongLong();

  // we should attach defaulttags!
  db.query("insert into appliedtags(tag, version)"
           " select tag, :a from defaulttags where folder==:b",
           vsn, parentid);
  
  return photo;
}

quint64 Scanner::addFolder(PhotoDB *db,
			   quint64 parentid, QString path, QString leaf) {
  // This is called from with thread or caller's thread.
  // Normally, a transaction should be in progress.
  QSqlQuery q =
    db->query("insert into folders(parentfolder,leafname,pathname) "
               " values (:a,:b,:c)", parentid?parentid:QVariant(),
               leaf, path);
  quint64 id = q.lastInsertId().toULongLong();

  if (parentid) {
    // update the foldertree
    db->query("insert into foldertree(descendant, ancestor) "
               " values (:a, :b)", id, parentid);
    db->query("insert into foldertree(descendant, ancestor) "
               " select :a, ancestor "
               " from foldertree where descendant==:b",
               id, parentid);

    // copy the defaulttags from parent
    db->query("insert into defaulttags(tag, folder) "
              " select tag, :a from defaulttags where folder==:b",
              id, parentid);
  }

  return id;
}

void Scanner::excludeTree(QString path) {
  removeTree(path);
  Untransaction t(db0);
  db0->query("insert into excludedtrees values (:a)", path);
}

void Scanner::removeTree(QString path) {
  // called from caller, not our thread
  Untransaction t(db0);
  db0->query("delete from folders where pathname==:a", path);
  db0->query("delete from excludedtrees where pathname like :a", path + "/%");

  // should clean the cache as well
}

void Scanner::run() {
  try {
    QMutexLocker l(&mutex);
    n = 0;
    N = photoQueueLength();
    m = 0;
    M = folderQueueLength(); // must actually maintain this
    while (!stopsoon) {
      bool sleepok = true;
      QSet<quint64> ids;
      if (!(ids=findFoldersToScan()).isEmpty()) {
	l.unlock();
        int N0 = N;
	scanFolders(ids);
        if (N>N0)
          emit collecting(N);
        sleepok = false;
	l.relock();
      }
      if (!(ids=findPhotosToScan()).isEmpty()) {
	l.unlock();
	scanPhotos(ids);
	pDebug() << "Scan progress: " << n << " / " << N;
	emit progressed(n, N);
        sleepok = false;
	l.relock();
      } else {
        l.unlock();
	if (N>0)
	  emit done();
	n = 0;
	N = 0;
        l.relock();
      }
      if (sleepok && !stopsoon) {
	waiter.wait(&mutex);
      }
    }
  } catch (QSqlQuery &q) {
    qDebug() << "Scanner: SqlError: " << q.lastError().text();
    qDebug() << "  from " << q.lastQuery();
    QMap<QString,QVariant> vv = q.boundValues();
    for (auto it=vv.begin(); it!=vv.end(); ++it) 
      qDebug() << "    " << it.key() << ": " << it.value();
    qDebug() << "  Thread terminating";
    emit exception("Scanner: SqlError: " + q.lastError().text()
		   + " from " + q.lastQuery());
  } catch (std::system_error &e) {
    qDebug() << "Scanner: System error: "
	     << e.code().value() << e.code().message().c_str();
    qDebug() << "  Thread terminating";
    emit exception("Scanner: System error");
  } catch (NoResult) {
    qDebug() << "Scanner: Expected object not found in table.";
    qDebug() << "  Thread terminating";
    emit exception("Scanner: No result");
  } catch (...) {
    qDebug() << "Scanner: Unknown exception";
    qDebug() << "  Thread terminating";
    emit exception("Scanner: Unknown exception");
  }
  pDebug() << "Scanner end of run";
}

QSet<quint64> Scanner::findPhotosToScan() {
  QSqlQuery qq = db.constQuery("select photo from photostoscan limit 100");
  QSet<quint64> ids;
  while (qq.next()) 
    ids << qq.value(0).toULongLong();
  return ids;
}
  
void Scanner::scanPhotos(QSet<quint64> ids) {
  QSet<quint64> versions;
  for (auto id: ids) {
    while (db.transactionsWaiting()) {
      pDebug() << "scanner waiting in photos";
      usleep(100000);
    }
    Transaction t(&db);
    //    pDebug() << "Scanner::scanPhotos: transaction started";
    scanPhoto(id);
    QSqlQuery q
      = db.constQuery("select id from versions where photo==:a", id);
    QSet<quint64> vv;
    while (q.next())
      vv << q.value(0).toULongLong();
    n++;
    t.commit();
    //    pDebug() << "Scanner::scanPhotos: transaction committed";
    if (!vv.isEmpty())
      emit updated(vv);
    versions |= vv;
  }

  if (!versions.isEmpty())
    emit updatedBatch(versions);
}

QSet<quint64> Scanner::findFoldersToScan() {
  QSqlQuery qq = db.constQuery("select folder from folderstoscan limit 30");
  QSet<quint64> ids;
  while (qq.next())
    ids << qq.value(0).toULongLong();
  return ids;
}

void Scanner::scanFolders(QSet<quint64> ids) {
  int N0 = N;
  //  bool worked = false;
  for (auto id: ids) {
    scanFolder(id);
    if (N >= N0 + 1000)
      break;
  }
}

void Scanner::scanFolder(quint64 id) {
  QString p = db.folder(id);
  QDir dir(p);

  QSet<QString> excludedtrees;
  QSet<QString> newsubdirs;
  QSet<QString> newphotos;
  QMap<QString, quint64> oldsubdirs;
  QMap<QString, quint64> oldphotos;
  QMap<QString, QDateTime> photodate;

  // Collect exclusions
  QSqlQuery q = db.query("select pathname from excludedtrees");
  while (q.next())
    excludedtrees.insert(q.value(0).toString());
  
  // Collect new subdirs and photos
  QFileInfoList infos(dir.entryInfoList(QDir::Dirs | QDir::Files
                                        | QDir::NoDotAndDotDot));
  for (auto i: infos) {
    if (i.isDir()) {
      if (!excludedtrees.contains(i.fileName()))
        newsubdirs << i.fileName();
    } else if (exts.contains(i.suffix().toLower())) {
      newphotos << i.fileName();
      photodate[i.fileName()] = i.lastModified();
    }
  }

  // Collect old subdirs
  q = db.query("select id, leafname from folders where parentfolder==:a", id);
  while (q.next())
    oldsubdirs[q.value(1).toString()] = q.value(0).toULongLong();

  // Collect old photos
  q = db.query("select id, filename from photos where folder==:a", id);
  while (q.next())
    oldphotos[q.value(1).toString()] = q.value(0).toULongLong();
  q.finish();
  
  // let's update the database

  while (db.transactionsWaiting()) {
    pDebug() << "Scanner waiting in folders";
    usleep(100000); 
  }
  Transaction t(&db);
  //  pDebug() << "Scanner::scanFolders: transaction started";
  db.query("delete from folderstoscan where folder=:a", id);
  
  // Drop subdirs that do not exist any more
  for (auto it=oldsubdirs.begin(); it!=oldsubdirs.end(); ++it) 
    if (!newsubdirs.contains(it.key())) 
      db.query("delete from folders where id==:a", it.value());

  // Drop photos that do not exist any more
  for (auto it=oldphotos.begin(); it!=oldphotos.end(); ++it) 
    if (!newphotos.contains(it.key())) 
      db.query("delete from photos where id==:a", it.value());

  // Insert newly found subdirs (and store IDs)
  for (auto s: newsubdirs) 
    if (!oldsubdirs.contains(s)) 
      oldsubdirs[s] = addFolder(&db, id, dir.path()+"/"+s, s);

  // Insert newly found photos
  for (auto s: newphotos)
    if (!oldphotos.contains(s))
      oldphotos[s] = addPhoto(id, s);

  // Add all existing folders to scan list
  for (auto s: newsubdirs)
    db.query("insert into folderstoscan values (:a)", oldsubdirs[s]);

  // Add new or modified photos to scan list
  for (auto s: newphotos) {
    QDateTime lastscan
      = db.simpleQuery("select lastscan from photos where id==:a",
		       oldphotos[s]).toDateTime();
    if (photodate[s]>lastscan || !lastscan.isValid()) {
      db.query("insert into photostoscan values (:a)", oldphotos[s]);
      N++;
    }
  }

  t.commit();
  //  pDebug() << "Scanner::scanFolders: transaction committed";
}

int Scanner::photoQueueLength() {
  return db.simpleQuery("select count(*) from photostoscan").toInt();
}

int Scanner::folderQueueLength() {
  return db.simpleQuery("select count(*) from folderstoscan").toInt();
}

void Scanner::scanPhoto(quint64 id) {
  // Do not create transaction: called from scanPhotos
  
  // Remove from queue
  db.query("delete from photostoscan where photo==:a", id);

  // Find the photo's filename on disk
  QSqlQuery q = db.query("select filename,folder from photos where id==:a", id);
  if (!q.next())
    throw NoResult();
  QString filename = q.value(0).toString();
  quint64 folder = q.value(1).toULongLong();
  q.finish();

  QString dirname = db.simpleQuery("select pathname from folders where id==:a",
				   folder).toString();
  QString pathname = dirname + "/" + filename;

  // Find exif info
  Exif exif(pathname);
  if (!exif.ok()) {
    qDebug() << "PhotoScanner::doscan: Could not get exif. Oh well.";
    return;
  }

  QDateTime captureDate =  exif.dateTime();
  if (!captureDate.isValid()) {
    // use file datestamp instead
    QFileInfo fileInfo(pathname);
    captureDate = fileInfo.lastModified();
  }    

  // Find camera, possibly creating new record
  QString model = exif.model();
  QString make = exif.make();
  quint64 camid = 0;
  if (!model.isNull()) {
    QSqlQuery q
      = db.query("select id from cameras where camera==:a and make==:b",
                 model, make);
    if (q.next()) {
      camid = q.value(0).toULongLong();
    } else {
      q = db.query("insert into cameras(camera, make) values(:a,:b)",
                   model, make);
      camid = q.lastInsertId().toULongLong();
    }
  }
  // Now camid is valid unless cam is empty

  QString lens = exif.lens();
  quint64 lensid = 0;
  if (!lens.isNull()) {
    QSqlQuery q = db.query("select id from lenses where lens==:a", lens);
    if (q.next()) {
      lensid = q.value(0).toULongLong();
    } else {
      q = db.query("insert into lenses(lens) values (:a)", lens);
      lensid = q.lastInsertId().toULongLong();
    }
  }
  // Now lensid is valid unless lens is empty

  q.prepare("update photos set "
            " width=:w, height=:h, camera=:c, lens=:l, orient=:or, "
            " exposetime=:e, fnumber=:f, focallength=:fl, "
            " distance=:d, iso=:iso, capturedate=:cd, "
            " lastscan=:ls where id==:id");
  q.bindValue(":w", exif.width());
  q.bindValue(":h", exif.height());
  q.bindValue(":c", model.isEmpty() ? QVariant() : QVariant(camid));
  q.bindValue(":l", lens.isEmpty() ? QVariant() : QVariant(lensid));
  q.bindValue(":or", int(exif.orientation()));
  q.bindValue(":e", exif.exposureTime_s());
  q.bindValue(":f", exif.fNumber());
  q.bindValue(":fl", exif.focalLength_mm());
  q.bindValue(":d", exif.focusDistance_m());
  q.bindValue(":iso", exif.iso());
  q.bindValue(":cd", captureDate);
  q.bindValue(":ls", QDateTime::currentDateTime());
  q.bindValue(":id", id);
  if (!q.exec())
    throw q;

#if 0
  QList<PSize> pvsiz = exif.previewSizes();
  if (!pvsiz.isEmpty()) {
    /* Using these thumbnails makes scanning very slow. I don't think
       it's worth it. Perhaps the cache process can somehow use
       them? */
  }
#endif
}
