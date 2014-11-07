// Scanner.cpp

#include "Scanner.h"
#include <QDebug>
#include <QDir>
#include <QFileInfoList>
#include <system_error>
#include <QDateTime>
#include "Exif.h"

class NoResult {
};

Scanner::Scanner(PhotoDB const &db): db(db) {
  QSqlQuery q(*db);
  q.prepare("select extension, filetype from extensions");
  if (!q.exec()) {
    qDebug() << "Could not select extensions";
    throw q.lastError();
  }
  while (q.next()) 
    exts[q.value(0).toString()] = q.value(1).toULongLong();
}

Scanner::~Scanner() {
  if (isRunning())
    stop();
  if (!wait(1000)) 
    qDebug() << "Failed to stop Scanner";
}

void Scanner::start() {
  if (!isRunning()) {
    stopsoon = false;
    QThread::start();
  }
}

void Scanner::stop() {
  if (isRunning()) {
    QMutexLocker l(&mutex);
    stopsoon = true;
    waiter.wakeOne();
  }
}

void Scanner::addTree(QString path) {
  // This is called from outside of thread!
  Transaction t(db);
  QSqlQuery q(*db);
  q.prepare("select id from folders where pathname==:p");
  q.bindValue(":p", path);
  if (!q.exec()) {
    qDebug() << "Could not select folder";
    throw q.lastError();
  }
  quint64 id;
  if (q.next()) {
    // folder exists
    id = q.value(0).toULongLong();
  } else {
    QDir parent(path);
    parent.cdUp();
    QString leaf = parent.relativeFilePath(path);
    QString parentPath = parent.path();
    q.prepare("select id from folders where pathname==:p");
    q.bindValue(":p", parentPath);
    if (!q.exec()) {
      qDebug() << "Could not select folder 2";
      throw q.lastError();
    }
    quint64 parentid = q.next() ? q.value(0).toULongLong() : 0;
    id = addFolder(parentid, path, leaf);
  }

  q.prepare("insert into folderstoscan values(:i)");
  q.bindValue(":i", id);
  if (!q.exec()) {
    qDebug() << "Could not insert into folderstoscan";
    throw q.lastError();
  }
  t.commit();

  QMutexLocker l(&mutex);
  waiter.wakeOne();
}

quint64 Scanner::addPhoto(quint64 parentid, QString leaf) {
  // This is called from inside of thread!
  QSqlQuery q(*db);

  // Insert into photo table
  q.prepare("insert into photos(folder, filename, filetype) "
	    " values (:p,:l,:t)");
  q.bindValue(":p", parentid);
  q.bindValue(":l", leaf);
  int idx = leaf.lastIndexOf(".");
  QString ext = leaf.mid(idx+1).toLower();
  if (!exts.contains(ext))
    q.bindValue(":t", QVariant()); // this should not happen
  else
    q.bindValue(":t", exts[ext]);
  if (!q.exec()) 
    throw q;

  quint64 id = q.lastInsertId().toULongLong();

  // Create first version - this is preliminary code
  q.prepare("insert into versions(photo) values(:i)");
  q.bindValue(":i", id);
  if (!q.exec()) 
    throw q;
  return id;
}

quint64 Scanner::addFolder(quint64 parentid, QString path, QString leaf) {
  QSqlQuery q(*db);
  q.prepare("insert or replace into folders(parentfolder,leafname,pathname) "
	    " values (:p,:l,:a)");
  if (parentid)
    q.bindValue(":p", parentid);
  else
    q.bindValue(":p", QVariant());
  q.bindValue(":l", leaf);
  q.bindValue(":a", path);
  if (!q.exec()) 
    throw q;
  quint64 id = q.lastInsertId().toULongLong();

  if (parentid) {
    q.prepare("insert into foldertree(descendant,ancestor) "
	      " values (:d,:a)");
    q.bindValue(":d", id);
    q.bindValue(":a", parentid);
    if (!q.exec())
      throw q;
    
    q.prepare("insert into foldertree(descendant,ancestor) "
	      " select ancestor, :d "
	      " from foldertree where descendant==:a");
    q.bindValue(":d", id);
    q.bindValue(":a", parentid);
    if (!q.exec()) 
      throw q;
  }
  return id;
}

void Scanner::removeTree(QString path) {
  Transaction t(db);
  QSqlQuery q(*db);
  q.prepare("delete from folders where pathname==:p");
  q.bindValue(":p", path);
  if (!q.exec())
    throw q;
  t.commit();
}

void Scanner::run() {
  n = 0;
  N = photoQueueLength();
  try {
    while (!stopsoon) {
      qDebug() << "Scanner: running";
      if (findFoldersToScan()) {
	// we don't report on folders
      } else if (findPhotosToScan()) {
	qDebug() << "Progress: " << n << " / " << N;
	emit progressed(n, N);
      } else {
	if (N>0)
	  emit done();
	n = 0;
	N = 0;
	mutex.lock();
	waiter.wait(&mutex);
	mutex.unlock();
      }
    }
  } catch (QSqlQuery &q) {
    qDebug() << "Scanner: SqlError: " << q.lastError().text();
    qDebug() << "  from " << q.lastQuery();
    QMap<QString,QVariant> vv = q.boundValues();
    for (auto it=vv.begin(); it!=vv.end(); ++it) 
      qDebug() << "    " << it.key() << ": " << it.value();
    qDebug() << "  Thread terminating";
  } catch (std::system_error &e) {
    qDebug() << "Scanner: System error: "
	     << e.code().value() << e.code().message().c_str();
    qDebug() << "  Thread terminating";
  } catch (NoResult) {
    qDebug() << "Scanner: Expected object not found in table.";
    qDebug() << "  Thread terminating";
  } catch (...) {
    qDebug() << "Scanner: Unknown exception";
    qDebug() << "  Thread terminating";
  }
}

bool Scanner::findPhotosToScan() {
  Transaction t(db);
  QSqlQuery qq(*db);
  qq.prepare("select photo from photostoscan limit 100");
  if (!qq.exec())
    throw qq;

  bool worked = false;
  while (qq.next()) {
    quint64 id = qq.value(0).toULongLong();
    scanPhoto(id);
    n++;
    worked = true;
  }

  if (worked) {
    t.commit();
    return true;
  } else {
    return false;
  }
}

bool Scanner::findFoldersToScan() {
  Transaction t(db);
  QSqlQuery qq(*db);
  qq.prepare("select folder from folderstoscan limit 100");
  if (!qq.exec())
    throw qq;
  int N0 = N;
  bool worked = false;
  while (N-N0<1000 && qq.next()) {
    // There's work to do
    quint64 id = qq.value(0).toULongLong();
    scanFolder(id);
    worked = true;
  }
  if (worked) {
    t.commit();
    return true;
  } else {
    return false;
  }
}

void Scanner::scanFolder(quint64 id) {
  QSqlQuery q(*db);

  q.prepare("delete from folderstoscan where folder=:i");
  q.bindValue(":i", id);
  if (!q.exec())
    throw q;

  q.prepare("select pathname from folders where id==:i");
  q.bindValue(":i", id);
  if (!q.exec())
    throw q;
  if (!q.next())
    throw NoResult();
  QDir dir(q.value(0).toString());
  qDebug() << "Scanning " << dir.path();

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
  q.prepare("select id, leafname from folders where parentfolder=:p");
  q.bindValue(":p", id);
  if (!q.exec())
    throw q;
  while (q.next())
    oldsubdirs[q.value(1).toString()] = q.value(0).toULongLong();

  // Collect old photos
  q.prepare("select id, filename from photos where folder=:p");
  q.bindValue(":p", id);
  if (!q.exec())
    throw q;
  while (q.next())
    oldphotos[q.value(1).toString()] = q.value(0).toULongLong();

  // Drop subdirs that do not exist any more
  for (auto it=oldsubdirs.begin(); it!=oldsubdirs.end(); ++it) {
    if (!newsubdirs.contains(it.key())) {
      q.prepare("delete from folders where id=:i");
      q.bindValue(":i", it.value());
    }
  }

  // Drop photos that do not exist any more
  for (auto it=oldphotos.begin(); it!=oldphotos.end(); ++it) {
    if (!newphotos.contains(it.key())) {
      q.prepare("delete from photos where id=:i");
      q.bindValue(":i", it.value());
    }
  }

  // Insert newly found subdirs (and store IDs)
  for (auto s: newsubdirs) 
    if (!oldsubdirs.contains(s)) 
      oldsubdirs[s] = addFolder(id, dir.path()+"/"+s, s);

  // Insert newly found photos
  for (auto s: newphotos)
    if (!oldphotos.contains(s))
      oldphotos[s] = addPhoto(id, s);

  // Add all existing folders to scan list
  for (auto s: newsubdirs) {
    q.prepare("insert into folderstoscan values(:i)");
    q.bindValue(":i", oldsubdirs[s]);
    if (!q.exec())
      throw q;
  }

  // Add new or modified photos to scan list
  for (auto s: newphotos) {
    q.prepare("select lastscan from photos where id=:i");
    q.bindValue(":i", oldphotos[s]);
    if (!q.exec())
      throw q;
    if (!q.next())
      throw NoResult();
    QDateTime lastscan = q.value(0).toDateTime();
    if (photodate[s]>lastscan || !lastscan.isValid()) {
      q.prepare("insert into photostoscan values(:i)");
      q.bindValue(":i", oldphotos[s]);
      if (!q.exec())
	throw q;
      N++;
    }
  }  
}

int Scanner::photoQueueLength() {
  QSqlQuery q(*db);
  q.prepare("select count(*) from photostoscan");
  if (!q.exec())
    throw q;
  if (!q.next())
    throw NoResult();
  return q.value(0).toInt();
}

void Scanner::scanPhoto(quint64 id) {
  QSqlQuery q(*db);

  // Remove from queue
  q.prepare("delete from photostoscan where photo=:i");
  q.bindValue(":i", id);
  if (!q.exec())
    throw q;

  // Find the photo's filename on disk
  q.prepare("select filename,folder from photos where id==:i");
  q.bindValue(":i", id);
  if (!q.exec())
    throw q;
  if (!q.next())
    throw NoResult();
  QString filename = q.value(0).toString();
  quint64 folder = q.value(1).toULongLong();
  q.prepare("select pathname from folders where id==:i");
  q.bindValue(":i", folder);
  if (!q.exec()) 
    throw q;
  if (!q.next())
    throw NoResult();
  QString dirname = q.value(0).toString();
  QString pathname = dirname + "/" + filename;

  // Find exif info
  Exif exif(pathname);
  if (!exif.ok()) {
    qDebug() << "PhotoScanner::doscan: Could not get exif. Oh well.";
    return;
  }

  // Find camera, possibly creating new record
  QString cam = exif.camera();
  quint64 camid = 0;
  if (!cam.isNull()) {
    q.prepare("select id from cameras where camera==:c");
    q.bindValue(":c", cam);
    if (!q.exec())
       throw q;
    if (q.next()) {
      camid = q.value(0).toULongLong();
    } else {
      q.prepare("insert into cameras(camera) values(:c)");
      q.bindValue(":c", cam);
      if (!q.exec())
	throw q;
      camid = q.lastInsertId().toULongLong();
    }
  }
  // Now camid is valid unless cam is empty

  QString lens = exif.lens();
  quint64 lensid = 0;
  if (!lens.isNull()) {
    q.prepare("select id from lenses where lens==:c");
    q.bindValue(":c", lens);
    if (!q.exec())
       throw q;
    if (q.next()) {
      lensid = q.value(0).toULongLong();
    } else {
      q.prepare("insert into lenses(lens) values(:c)");
      q.bindValue(":c", lens);
      if (!q.exec())
	throw q;
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
  q.bindValue(":c", cam.isEmpty() ? QVariant() : QVariant(camid));
  q.bindValue(":l", lens.isEmpty() ? QVariant() : QVariant(lensid));
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
}
