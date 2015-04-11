// Scanner.cpp

#include "Scanner.h"
#include "PDebug.h"
#include <QDir>
#include <QFileInfoList>
#include <system_error>
#include <QDateTime>
#include "Exif.h"
#include "NoResult.h"

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

void Scanner::addTree(QString path) {
  // This is called from outside of thread!
  Transaction t(db0);
  QSqlQuery q = db0->constQuery("select id from folders where pathname==:a",
			      path);
  quint64 id;
  if (q.next()) {
    // folder exists
    id = q.value(0).toULongLong();
  } else {
    QDir parent(path);
    parent.cdUp();
    QString leaf = parent.relativeFilePath(path);
    QString parentPath = parent.path();
    QSqlQuery q
      = db0->constQuery("select id from folders where pathname==:a",
                        parentPath);
    quint64 parentid = q.next() ? q.value(0).toULongLong() : 0;
    id = addFolder(parentid, path, leaf);
  }

  db0->query("insert into folderstoscan values (:a)", id);
  t.commit();

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
  quint64 id = q.lastInsertId().toULongLong();

  // Create first version - this is preliminary code
  db.query("insert into versions(photo) values(:i)", id);

  return id;
}

quint64 Scanner::addFolder(quint64 parentid, QString path, QString leaf) {
  QSqlQuery q =
    db0->query("insert into folders(parentfolder,leafname,pathname) "
               " values (:a,:b,:c)", parentid?parentid:QVariant(),
               leaf, path);
  quint64 id = q.lastInsertId().toULongLong();

#if 0
  if (parentid) {
    db0->query("insert into foldertree(descendant, ancestor) "
               " values (:a, :b)", id, parentid);
    db0->query("insert into foldertree(descendant, ancestor) "
               " select :a, ancestor "
               " from foldertree where descendant==:b",
               id, parentid);
  }
#endif
  return id;
}

void Scanner::removeTree(QString path) {
  // called from caller, not our thread
  Transaction t(db0);
  db0->query("delete from folders where pathname==:a", path);
  t.commit();
}

void Scanner::run() {
  try {
    QMutexLocker l(&mutex);
    n = 0;
    N = photoQueueLength();
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
  Transaction t(&db);
  bool worked = false;
  QSet<quint64> versions;
  for (auto id: ids) {
    scanPhoto(id);
    QSqlQuery q
      = db.constQuery("select id from versions where photo==:a", id);
    while (q.next())
      versions << q.value(0).toULongLong();
    n++;
    worked = true;
  }

  if (worked) {
    t.commit();
    emit updated(versions);
  }
}

QSet<quint64> Scanner::findFoldersToScan() {
  QSqlQuery qq = db.constQuery("select folder from folderstoscan limit 30");
  QSet<quint64> ids;
  while (qq.next())
    ids << qq.value(0).toULongLong();
  return ids;
}

void Scanner::scanFolders(QSet<quint64> ids) {
  Transaction t(&db);
  int N0 = N;
  bool worked = false;
  for (auto id: ids) {
    // There's work to do
    scanFolder(id);
    worked = true;
    if (N >= N0 + 1000)
      break;
  }
  if (worked) 
    t.commit();
}

void Scanner::scanFolder(quint64 id) {
  // do not create transaction: called from scanFolders
  db.query("delete from folderstoscan where folder=:a", id);
  QString p = db.simpleQuery("select pathname from folders where id==:a", id)
    .toString();
  QDir dir(p);

  QSet<QString> newsubdirs;
  QSet<QString> newphotos;
  QMap<QString, quint64> oldsubdirs;
  QMap<QString, quint64> oldphotos;
  QMap<QString, QDateTime> photodate;

  // Collect new subdirs and photos
  QFileInfoList infos(dir.entryInfoList(QDir::Dirs | QDir::Files
                                        | QDir::NoDotAndDotDot));
  for (auto i: infos) {
    if (i.isDir()) {
      newsubdirs << i.fileName();
    } else if (exts.contains(i.suffix().toLower())) {
      newphotos << i.fileName();
      photodate[i.fileName()] = i.lastModified();
    }
  }

  // Collect old subdirs
  QSqlQuery q
    = db.query("select id, leafname from folders where parentfolder==:a", id);
  while (q.next())
    oldsubdirs[q.value(1).toString()] = q.value(0).toULongLong();

  // Collect old photos
  q = db.query("select id, filename from photos where folder==:a", id);
  while (q.next())
    oldphotos[q.value(1).toString()] = q.value(0).toULongLong();

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
      oldsubdirs[s] = addFolder(id, dir.path()+"/"+s, s);

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
}

int Scanner::photoQueueLength() {
  return db.simpleQuery("select count(*) from photostoscan").toInt();
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

  QString dirname = db.simpleQuery("select pathname from folders where id==:a",
				   folder).toString();
  QString pathname = dirname + "/" + filename;

  // Find exif info
  Exif exif(pathname);
  if (!exif.ok()) {
    qDebug() << "PhotoScanner::doscan: Could not get exif. Oh well.";
    return;
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
      QSqlQuery q = db.query("insert into cameras(camera, make) values(:a,:b)",
			     model, make);
      camid = q.lastInsertId().toULongLong();
    }
  }
  // Now camid is valid unless cam is empty

  QString lens = exif.lens();
  pDebug() << "Scanner: " << make << model << lens;
  quint64 lensid = 0;
  if (!lens.isNull()) {
    QSqlQuery q = db.query("select id from lenses where lens==:a", lens);
    if (q.next()) {
      lensid = q.value(0).toULongLong();
    } else {
      QSqlQuery q = db.query("insert into lenses(lens) values (:a)", lens);
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
  q.bindValue(":cd", exif.dateTime()); // does this work?
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
    q.prepare("select id from versions where photo==:i and mods is null");
    q.bindValue(":i", id);
    if (q.exec() && q.next()) {
      quint64 vsn = q.value(0).toULongLong();
      PSize maxs;
      int npix = 0;
      for (int n=0; n<pvsiz.size(); n++) {
        PSize s = pvsiz[n];
        int np = s.width()*s.height();
        if (np>npix && np<100*1000) {
          maxs = s;
          npix = np;
        }
      }

      if (npix>0) {
        Image16 img = exif.previewImage(maxs);
        emit cacheablePreview(vsn, img);
      }
    }
  }
#endif
}
