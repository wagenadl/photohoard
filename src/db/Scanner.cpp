// Scanner.cpp

#include "Scanner.h"
#include "PDebug.h"
#include <QDir>
#include <QFileInfoList>
#include <system_error>
#include <QDateTime>
#include "Exif.h"
#include "Tags.h"
#include "Here.h"
#include <QUrl>

Scanner::Scanner(SessionDB *db0): db0(db0) {
  setObjectName("Scanner");
  db.clone(*db0);
  QSqlQuery q = db0->constQuery("select extension, filetype from extensions");
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

QStringList Scanner::allRoots() {
  QSqlQuery q = db0->query("select pathname from folders"
                           " where parentfolder is null");
  QStringList roots;
  while (q.next())
    roots << q.value(0).toString();
  
  return roots;
}

void Scanner::addSubTree(QString folder) {
  // Called from outside of thread
  QDir dir(folder);
  QString path = dir.absolutePath();
  if (db0->constQuery("select id from folders where pathname==:a", path)
      .next()) {
    // got it already -> easy
    rescan(path);
    return;
  }
  QString npath;
  while (true) {
    dir.cdUp();
    npath = dir.absolutePath();
    if (npath==path) {
      COMPLAIN("Scanner::addSubTree: " + folder
               + " is not a descendent of any root in the db");
      return; // not found
    }

    QSqlQuery q = db0->constQuery("select id from folders where pathname==:a",
                                  npath);
    if (q.next())
      break;
    path = npath;
  }

  // Now, npath is a folder that exists in the db, and path is a subfolder
  // that doesn't yet.
  QString coll = db0->defaultQuery("select tags.tag from tags"
                                   " inner join defaulttags"
                                   " on tags.id==defaulttags.tag"
                                   " inner join folders"
                                   " on defaulttags.folder==folders.id"
                                   " where folders.pathname==:a", npath, "")
    .toString();
  addTree(path, coll);
}
   

void Scanner::rescanAll() {
  // This is called from outside of thread!
  QStringList roots = allRoots();
  for (QString r: roots)
    rescan(r);
}

void Scanner::rescan(QString path) {
  // This is called from outside of thread!
  Untransaction t(db0);
  db0->query("insert into folderstoscan select id from folders"
             " where pathname==:a", path);

  QMutexLocker l(&mutex);
  waiter.wakeOne();
}

quint64 Scanner::addPhoto(quint64 parentid, QString leaf) {
  // This is called from inside of thread!
  // Insert into photo table
  // Many details about the photo will be updated after scanning
  int idx = leaf.lastIndexOf(".");
  QString ext = leaf.mid(idx+1).toLower();

  QSqlQuery q
    = db.query("insert into photos(folder, filename, filetype) "
	       " values (:a,:b,:c)",
	       parentid, leaf, exts.contains(ext) ? exts[ext]: QVariant());
  quint64 photo = q.lastInsertId().toULongLong();

  // Create first version
  q = db.query("insert into versions(photo,acceptreject)"
	       " values(:a,:b)", photo,int(PhotoDB::AcceptReject::NewImport));
  quint64 vsn = q.lastInsertId().toULongLong();

  // Attach default tags
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
      scanFolders(ids);
      sleepok = false;
      l.relock();
    }
    if (!(ids=findPhotosToScan()).isEmpty()) {
      l.unlock();
      scanPhotos(ids);
      sleepok = false;
      l.relock();
    }
    if (sleepok && (N>0 || M>0)) {
      l.unlock();
      reportScanDone();
      l.relock();
      n = 0;
      N = 0;
      m = 0;
      M = 0;
    }
    if (sleepok && !stopsoon) {
      waiter.wait(&mutex);
    }
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
  int n0 = n;
  for (auto id: ids) {
    while (db.transactionsWaiting()) {
      pDebug() << "scanner waiting in photos";
      usleep(100000);
    }

    Transaction t(&db);
    scanPhoto(id);
    QSqlQuery q
      = db.constQuery("select id from versions where photo==:a", id);
    QSet<quint64> vv;
    while (q.next())
      vv << q.value(0).toULongLong();
    n++;
    t.commit();

    if (n>=n0+5) {
      reportPhotoProgress();
      n0 = n;
    }
    
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
  int M0 = M;
  int m0 = m;
  QSet<QString> excludedtrees;
  // Collect exclusions
  { QSqlQuery q = db.query("select pathname from excludedtrees");
    while (q.next())
      excludedtrees.insert(q.value(0).toString());
  }

  for (auto id: ids) {
    scanFolder(id, excludedtrees);
    if (m>=m0+5 || M>=M0+5) {
      reportFolderProgress();
      m0 = m;
      M0 = M;
    }
    if (N >= N0 + 1000)
      break;
  }
}

void Scanner::scanFolder(quint64 id, QSet<QString> const &excludedtrees) {
  QString p = db.folder(id);
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
      if (!excludedtrees.contains(i.fileName()))
        newsubdirs << i.fileName();
    } else if (exts.contains(i.suffix().toLower())) {
      newphotos << i.fileName();
      photodate[i.fileName()] = i.lastModified();
    }
  }

  QSqlQuery q;
  
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
  db.query("delete from folderstoscan where folder=:a", id);
  m++;
  
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
  for (auto s: newsubdirs) {
    db.query("insert into folderstoscan values (:a)", oldsubdirs[s]);
    M++;
  }
  
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
  ASSERT(q.next());
  QString filename = q.value(0).toString();
  quint64 folder = q.value(1).toULongLong();
  q.finish();

  QString dirname = db.simpleQuery("select pathname from folders where id==:a",
				   folder).toString();
  QString pathname = dirname + "/" + filename;

  // Find exif info
  Exif exif(pathname);
  if (!exif.ok()) {
    COMPLAIN("PhotoScanner::doscan: Could not get exif. Oh well.");
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
            " width=:w, height=:h, camera=:c, lens=:l, "
            " exposetime=:e, fnumber=:f, focallength=:fl, "
            " distance=:d, iso=:iso, capturedate=:cd, "
            " lastscan=:ls where id==:id");
  q.bindValue(":w", exif.width());
  q.bindValue(":h", exif.height());
  q.bindValue(":c", model.isEmpty() ? QVariant() : QVariant(camid));
  q.bindValue(":l", lens.isEmpty() ? QVariant() : QVariant(lensid));
  //  q.bindValue(":or", int(exif.orientation()));
  q.bindValue(":e", exif.exposureTime_s());
  q.bindValue(":f", exif.fNumber());
  q.bindValue(":fl", exif.focalLength_mm());
  q.bindValue(":d", exif.focusDistance_m());
  q.bindValue(":iso", exif.iso());
  q.bindValue(":cd", captureDate);
  q.bindValue(":ls", QDateTime::currentDateTime());
  q.bindValue(":id", id);
  ASSERT(q.exec());

  db.query("update versions set orient=:a where photo==:b",
	   int(exif.orientation()), id);

#if 0
  QList<PSize> pvsiz = exif.previewSizes();
  if (!pvsiz.isEmpty()) {
    /* Using these thumbnails makes scanning very slow. I don't think
       it's worth it. Perhaps the cache process can somehow use
       them? */
  }
#endif
}


void Scanner::reportPhotoProgress() {
  pDebug() << "Photo scan progress: " << n << " / " << N;
  QString msg = QString("Scanning photos: %1/%2").arg(n).arg(N);
  emit message(msg);
}

void Scanner::reportFolderProgress() {
  pDebug() << "Folder scan progress: " << m << " / " << M;
  QString msg = QString("Scanning folders: %1/%2").arg(m).arg(M);
  emit message(msg);
}

void Scanner::reportScanDone() {
  pDebug() << "Scan complete";
  emit message("Scan complete");
}

//////////////////////////////////////////////////////////////////////
void Scanner::importDragged(QList<QUrl> urls, QString coll) {
  for (auto const &url: urls) {
    ASSERT(url.isLocalFile());
    QFileInfo fi(url.path());
    if (fi.isDir())
      qDebug() << "  Import folder" << fi.absoluteFilePath();
    else
      qDebug() << "  Import photo" << fi.absoluteFilePath();
  }
  qDebug() << "  Into collection: " << coll;
}
