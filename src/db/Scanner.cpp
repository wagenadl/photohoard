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
  DBReadLock lock(db0);
  QSqlQuery q = db0->constQuery("select extension, filetype from extensions");
  while (q.next()) 
    exts[q.value(0).toString()] = q.value(1).toInt();
}

Scanner::~Scanner() {
  stopAndWait();
  db.close();
}

quint64 Scanner::findDirOrAdd(QString path, bool secondary) {
  quint64 id = db0->findFolder(path);
  if (id)
    return id;
  if (secondary && excludedTrees().contains(path))
    return 0;
  QDir parent(path);
  parent.cdUp();
  QString parentpath = parent.path();
  quint64 parentid = (parentpath==path) ? 0 : findDirOrAdd(parentpath, true);
  bool primary = !secondary;
  if (primary || parentid) {
    QDir dir(path);
    id = addFolder(db0, parentid, path, parent.relativeFilePath(path));
  }
  return id;
}

void Scanner::addTree(QString path, QString defaultCollection,
                      QStringList excluded) {
  // This is called from outside of thread!
  { DBWriteLock lock(db0);
    db0->query("delete from excludedtrees where pathname==:a", path);
  }

  foreach (QString e, excluded)
    excludeTree(e);
  
  if (!db0->findFolder(path)) {
    // We don't have the folder yet
    quint64 id = findDirOrAdd(path);
    if (!defaultCollection.isEmpty()) {
      Tags tags(db0);
      int t = tags.ensureCollection(defaultCollection);
      Transaction tra(&db);
      db0->query("delete from defaulttags where folder==:a", id);
      db0->query("insert into defaulttags(folder, tag) values(:a,:b)", id, t);
      tra.commit();
    }
  }

  rescan(path);
}

QStringList Scanner::allRoots() {
  DBReadLock lock(db0);
  QSqlQuery q = db0->constQuery("select pathname from folders"
                           " where parentfolder is null");
  QStringList roots;
  while (q.next())
    roots << q.value(0).toString();
  
  return roots;
}

void Scanner::rescanAll() {
  // This is called from outside of thread!
  QStringList roots = allRoots();
  for (QString r: roots)
    rescan(r);
}

void Scanner::rescan(QString path) {
  // This is called from outside of thread!
  { DBWriteLock lock(db0);
    db0->query("insert into folderstoscan select id from folders"
               " where pathname==:a", path);
  }
  QMutexLocker l(&mutex);
  waiter.wakeOne();
}

quint64 Scanner::addPhoto(quint64 parentid, QString leaf) {
  // This is called from inside of thread!
  // Insert into photo table
  // Many details about the photo will be updated after scanning
  // Must be called within a transaction
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
  Transaction t(db);
  quint64 id = db->query("insert into"
                         " folders(parentfolder, leafname, pathname) "
                         " values (:a,:b,:c)",
                         parentid ? parentid : QVariant(), leaf, path)
    .lastInsertId().toULongLong();

  if (parentid) {
    // copy the defaulttags from parent
    db->query("insert into defaulttags(tag, folder) "
              " select tag, :a from defaulttags where folder==:b",
              id, parentid);
  }

  /* Now let's see if any folders that were hitherto roots are in fact
     direct children of this new folder */
  QSqlQuery q = db->constQuery("select id, pathname, leafname from folders"
                          " where parentfolder is null"
                          " and pathname like :a", path + "/%");
  while (q.next()) {
    quint64 cid = q.value(0).toULongLong();
    QString cpath = q.value(1).toString();
    QString cleaf = q.value(2).toString();
    if (path + "/" + cleaf == cpath) 
      db->query("update folders set parentfolder=:a where id==:b", id, cid);
  }
  t.commit();
  return id;
}

void Scanner::excludeTree(QString path) {
  removeTree(path);
  DBWriteLock lock(db0);

  db0->query("insert into excludedtrees values (:a)", path);
}

void Scanner::removeTree(QString path) {
  // called from caller, not our thread
  DBWriteLock lock(db0);
  
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
  //  pDebug() << "Scanner end of run";
}

QSet<quint64> Scanner::findPhotosToScan() {
  DBReadLock lock(&db);
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

    scanPhoto(id);
    QSet<quint64> vv;
    { DBReadLock lock(&db);
      QSqlQuery q = db.constQuery("select id from versions where photo==:a",
                                  id);
      while (q.next())
        vv << q.value(0).toULongLong();
      n++;
    }

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
  DBReadLock lock(&db);
  QSqlQuery qq = db.constQuery("select folder from folderstoscan limit 30");
  QSet<quint64> ids;
  while (qq.next())
    ids << qq.value(0).toULongLong();
  return ids;
}

QSet<QString> Scanner::excludedTrees() {
  DBReadLock lock(&db);
  QSet<QString> trees;
  QSqlQuery q = db.constQuery("select pathname from excludedtrees");
  while (q.next())
    trees.insert(q.value(0).toString());
  return trees;
}

void Scanner::scanFolders(QSet<quint64> ids) {
  int N0 = N;
  int M0 = M;
  int m0 = m;
  QSet<QString> excludedtrees = excludedTrees();

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

QMap<QString, quint64> Scanner::photosInFolder(quint64 folderid) const {
  QMap<QString, quint64> photos;
  DBReadLock lock(&db);
  QSqlQuery q = db.constQuery("select id, filename from photos"
                              " where folder==:a", folderid);
  while (q.next())
    photos[q.value(1).toString()] = q.value(0).toULongLong();
  return photos;
}

QMap<QString, quint64> Scanner::subFolders(quint64 folderid) const {
  QMap<QString, quint64> folders;
  DBReadLock lock(&db);
  QSqlQuery q = db.constQuery("select id, leafname from folders"
                              " where parentfolder==:a", folderid);
  while (q.next())
    folders[q.value(1).toString()] = q.value(0).toULongLong();
  return folders;
}

void Scanner::dropPhotos(QList<quint64> photoids) {
  // delete vanished photos from db
  if (photoids.isEmpty())
    return;
  Transaction t(&db);
  for (quint64 id: photoids)
    db.query("delete from photos where id==:a", id);
  t.commit();
}

void Scanner::dropPhotosInFolder(quint64 folderid) {
  dropPhotos(photosInFolder(folderid).values());
  QMap<QString, quint64> subfolders = subFolders(folderid);
  for (quint64 id: subfolders)
    dropPhotosInFolder(id);
}

void Scanner::dropFolders(QList<quint64> folderids) {
  if (folderids.isEmpty())
    return;

  // remove sub folders, recursively, first
  for (quint64 id: folderids)
    dropFolders(subFolders(id).values());

  Transaction t(&db);
  for (quint64 id: folderids) {
    db.query("delete from folderstoscan where folder=:a", id);
    db.query("delete from folders where id==:a", id);
  }
  t.commit();
}
  
void Scanner::scanFolder(quint64 id, QSet<QString> const &excludedtrees) {
  QString p = db.folder(id);
  QDir dir(p);

  QMap<QString, quint64> oldsubdirs = subFolders(id);
  QMap<QString, quint64> oldphotos = photosInFolder(id);

  // Collect new subdirs and photos
  QSet<QString> newsubdirs;
  QSet<QString> newphotos;
  QMap<QString, QDateTime> photodate;
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

  QList<quint64> dropphotos;
  for (auto it=oldphotos.begin(); it!=oldphotos.end(); ++it) 
    if (!newphotos.contains(it.key())) 
      dropphotos << it.value();
  dropPhotos(dropphotos);

  QList<quint64> dropsubdirs;
  for (auto it=oldsubdirs.begin(); it!=oldsubdirs.end(); ++it) 
    if (!newsubdirs.contains(it.key())) 
      dropsubdirs << it.value();
  dropFolders(dropsubdirs);

  // Insert newly found subdirs (and store IDs)
  for (auto s: newsubdirs) 
    if (!oldsubdirs.contains(s)) 
      oldsubdirs[s] = addFolder(&db, id, dir.path()+"/"+s, s);

  { Transaction t(&db);
    // Insert newly found photos
    for (auto s: newphotos)
      if (!oldphotos.contains(s))
        oldphotos[s] = addPhoto(id, s);
    // remove this dir from scan list
    db.query("delete from folderstoscan where folder=:a", id);
    t.commit();
  }

  if (newsubdirs.size()) {
    Transaction t(&db);
    // Add all existing subfolders to scan list
    for (auto s: newsubdirs) {
      db.query("insert into folderstoscan values (:a)", oldsubdirs[s]);
      M++;
    }
    t.commit();
  }
  
  // Add new or modified photos to scan list
  if (newphotos.size()) {
    Transaction t(&db);
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
}

int Scanner::photoQueueLength() {
  DBReadLock lock(&db);
  return db.simpleQuery("select count(*) from photostoscan").toInt();
}

int Scanner::folderQueueLength() {
  DBReadLock lock(&db);
  return db.simpleQuery("select count(*) from folderstoscan").toInt();
}

void Scanner::scanPhoto(quint64 id) {
  { DBWriteLock lock(&db);

    // Remove from queue
    db.query("delete from photostoscan where photo==:a", id);
  }

  // Find the photo's filename on disk
  QString filename;
  quint64 folder;
  { DBReadLock lock(&db);
    QSqlQuery q = db.constQuery("select filename,folder from photos where id==:a", id);
    ASSERT(q.next());
    filename = q.value(0).toString();
    folder = q.value(1).toULongLong();
  }

  QString dirname;
  QString pathname;
  { DBReadLock lock(&db);
    dirname = db.simpleQuery("select pathname from folders where id==:a",
				   folder).toString();
    pathname = dirname + "/" + filename;
  }
  
  // Find exif info
  Exif exif(pathname);
  if (!exif.ok()) {
    COMPLAIN("PhotoScanner::doscan: Could not get exif. Oh well.");
    return;
  }

  QDateTime captureDate =  exif.dateTime();
  if (!captureDate.isValid() || captureDate.date().year() == 1980) {
    // use file datestamp instead
    QFileInfo fileInfo(pathname);
    captureDate = fileInfo.lastModified();
    captureDate.setTime(QTime()); // set to midnight to mark trouble
  }    

  // Find camera, possibly creating new record
  QString model = exif.model();
  QString make = exif.make();
  quint64 camid = 0;
  if (!model.isEmpty()) {
    { DBReadLock lock(&db);
      QSqlQuery q
        = db.constQuery("select id from cameras where camera==:a and make==:b",
                        model, make);
      if (q.next()) 
        camid = q.value(0).toULongLong();
    }
    if (!camid) {
      DBWriteLock lock(&db);

      QSqlQuery q = db.query("insert into cameras(camera, make) values(:a,:b)",
                             model, make);
      camid = q.lastInsertId().toULongLong();
    }
  }
  // Now camid is valid unless cam is empty

  QString lens = exif.lens();
  quint64 lensid = 0;
  if (!lens.isEmpty()) {
    { DBReadLock lock(&db);
      QSqlQuery q = db.constQuery("select id from lenses where lens==:a", lens);
      if (q.next()) 
        lensid = q.value(0).toULongLong();
    }
    if (!lensid) {
      DBWriteLock lock(&db);

      QSqlQuery q = db.query("insert into lenses(lens) values (:a)", lens);
      lensid = q.lastInsertId().toULongLong();
    }
  }
  // Now lensid is valid unless lens is empty

  { QSqlQuery q = db.query();
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
    DBWriteLock lock(&db);
    if (!q.exec()) {
      pDebug() << "fail!" << q.lastQuery() << q.boundValues();
      CRASH(q.lastError().text());
    }

    db.query("update versions set orient=:a where photo==:b",
             int(exif.orientation()), id);
  }
}


void Scanner::reportPhotoProgress() {
  //  pDebug() << "Photo scan progress: " << n << " / " << N;
  QString msg = QString("Scanning photos: %1/%2").arg(n).arg(N);
  emit message(msg);
}

void Scanner::reportFolderProgress() {
  //  pDebug() << "Folder scan progress: " << m << " / " << M;
  QString msg = QString("Scanning folders: %1/%2").arg(m).arg(M);
  emit message(msg);
}

void Scanner::reportScanDone() {
  //  pDebug() << "Scan complete";
  emit message("Scan complete");
}

/*/////////////////////////////////////////////////////////////////////
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
*/
