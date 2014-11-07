// PhotoScanner.cpp

#include "PhotoScanner.h"

#include <QFileInfoList>
#include <QSqlQuery>
#include <QDebug>
#include <QMutexLocker>
#include <QSqlError>
#include <QDir>
#include "Exif.h"

PhotoScanner::PhotoScanner(QSqlDatabase const &db, class AutoCache *):
  db(db) {
  QSqlQuery q;
  q.prepare("select extension, filetype from extensions");
  if (!q.exec()) {
    qDebug() << "Could not select extensions";
    return;
  }
  while (q.next()) 
    exts[q.value(0).toString()] = q.value(1).toULongLong();
}

PhotoScanner::~PhotoScanner() {
  if (isRunning())
    stop();
  if (!wait(1000)) 
    qDebug() << "Failed to stop PhotoScanner";
}

void PhotoScanner::start() {
  if (!isRunning()) {
    stopsoon = false;
    QThread::start();
  }
}

void PhotoScanner::stop() {
  if (isRunning()) {
    QMutexLocker l(&mutex);
    stopsoon = true;
    waiter.wakeOne();
  }
}

void PhotoScanner::rescan(quint64 photo, bool internal) {
  QSqlQuery q(db);
  q.prepare("insert into photostoscan values(:i)");
  q.bindValue(":i", photo);
  if (!q.exec())
    qDebug() << "PhotoScanner::rescan: " << q.lastError().text();
  if (!internal) {
    QMutexLocker l(&mutex);
    waiter.wakeOne();
  }
}

bool PhotoScanner::add(quint64 folder, QString leafname, bool internal) {
  int idx = leafname.lastIndexOf(".");
  if (idx<0)
    return false;
  QString ext = leafname.mid(idx+1).toLower();
  if (!exts.contains(ext))
    return false;

  QSqlQuery q(db);

  if (!q.exec("begin transaction")) {
    qDebug() << "PhotoScanner::add: Could not begin transaction";
    return false;
  }
  q.prepare("insert into photos(folder, filename, filetype) "
              " values(:f,:n,:t)");
  q.bindValue(":f", folder);
  q.bindValue(":n", leafname);
  q.bindValue(":t", exts[ext]);
  if (!q.exec()) {
    qDebug() << "  Could not insert photo";
    if (!q.exec("rollback transaction"))
      qDebug() << "  Could not roll back transaction";
    return false;
  }
  quint64 id = q.lastInsertId().toULongLong();

  q.prepare("insert into versions(photo) values(:i)");
  q.bindValue(":i", id);
  if (!q.exec()) {
    qDebug() << "  Could not insert version";
    if (!q.exec("rollback transaction"))
      qDebug() << "  Could not roll back transaction";
    return false;
  }
  if (!q.exec("commit transaction")) {
    qDebug() << "  Could not commit transaction";
    return false;
  }
  
  rescan(id, internal);
  return true;
}

void PhotoScanner::run() {
  while (!stopsoon) {
    qDebug() << "PhotoScanner: running";
    QSqlQuery q(db);
    q.prepare("select photo from photostoscan limit 1");
    mutex.lock();
    if (!q.exec()) {
      mutex.unlock();
      qDebug() << "PhotoScanner: Could not run select query";
      quit();
    }
    if (q.next()) {
      mutex.unlock();
      quint64 id = q.value(0).toULongLong();
      doscan(id);
      q.prepare("delete from photostoscan where photo=:i");
      q.bindValue(":i", id);
      if (!q.exec()) {
        qDebug() << "PhotoScanner: Could not remove from scanlist";
        quit();
      }
    } else {
      qDebug() << "PhotoScanner: Waiting";
      waiter.wait(&mutex);
      mutex.unlock();
    }
  }    
}

void PhotoScanner::doscan(quint64 photo) {
  QSqlQuery q(db);
  q.prepare("select filename,folder from photos where id==:i");
  q.bindValue(":i", photo);
  if (!q.exec()) {
    qDebug() << "PhotoScanner::doscan: Failed to select filename";
    quit();
  }
  if (!q.next()) {
    qDebug() << "PhotoScanner::doscan: Photo not found: " << photo;
    return;
  }
  QString filename = q.value(0).toString();
  quint64 folder = q.value(1).toULongLong();
  qDebug() << "Scanning " << folder << ":" << filename;
  q.prepare("select pathname from folders where id==:i");
  q.bindValue(":i", folder);
  if (!q.exec()) {
    qDebug() << "PhotoScanner::doscan: Failed to select folder";
    quit();
  }
  if (!q.next()) {
    qDebug() << "PhotoScanner::doscan: Folder not found: " << folder;
    return;
  }
  QString dirname = q.value(0).toString();

  QString pathname = dirname + "/" + filename;
  
  Exif exif(pathname);
  if (!exif.ok()) {
    qDebug() << "PhotoScanner::doscan: Could not get exif. Oh well.";
    return;
  }

  QString cam = exif.camera();
  quint64 camid = 0;
  if (!cam.isNull()) {
    q.prepare("select id from cameras where camera==:c");
    q.bindValue(":c", cam);
    if (q.exec() && q.next()) {
      camid = q.value(0).toULongLong();
    } else {
      q.prepare("insert into cameras(camera) values(:c)");
      q.bindValue(":c", cam);
      if (q.exec()) {
        camid = q.lastInsertId().toULongLong();
      } else {
        qDebug() << "PhotoScanner::doscan: Could not define camera";
        return;
      }
    }
  }
  // now camid is valid unless cam is empty


  QString lens = exif.lens();
  quint64 lensid = 0;
  if (!lens.isNull()) {
    q.prepare("select id from lenses where lens==:c");
    q.bindValue(":c", lens);
    if (q.exec() && q.next()) {
      lensid = q.value(0).toULongLong();
    } else {
      q.prepare("insert into lenses(lens) values(:c)");
      q.bindValue(":c", lens);
      if (q.exec()) {
        lensid = q.lastInsertId().toULongLong();
      } else {
        qDebug() << "PhotoScanner::doscan: Could not define lens";
        return;
      }
    }
  }
  // now lensid is valid unless lens is empty

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
  q.bindValue(":id", photo);
  if (!q.exec()) {
    qDebug() << "  Could not update photo";
    qDebug() << "    " << q.lastError().text();
    return;
  }
}
