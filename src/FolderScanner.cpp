// FolderScanner.cpp

#include "FolderScanner.h"
#include <QFileInfoList>
#include <QSqlQuery>
#include <QDebug>
#include <QMutexLocker>
#include <QSqlError>
#include <QDir>

FolderScanner::FolderScanner(QSqlDatabase const &db, PhotoScanner *phscan):
  db(db), phscan(phscan) {
}

FolderScanner::~FolderScanner() {
  if (isRunning())
    stop();
  if (!wait(1000)) 
    qDebug() << "Failed to stop FolderScanner";
}

void FolderScanner::start() {
  if (!isRunning()) {
    stopsoon = false;
    QThread::start();
  }
}

void FolderScanner::stop() {
  if (isRunning()) {
    QMutexLocker l(&mutex);
    stopsoon = true;
    waiter.wakeOne();
  }
}

void FolderScanner::rescan(quint64 folder, bool internal) {
  QSqlQuery q(db);
  q.prepare("insert into folderstoscan values(:i)");
  q.bindValue(":i", folder);
  if (!q.exec())
    qDebug() << "FolderScanner::rescan: " << q.lastError().text();
  if (!internal) {
    QMutexLocker l(&mutex);
    waiter.wakeOne();
  }
}

void FolderScanner::add(QString dirpath, bool internal) {
  QSqlQuery q(db);
  QDir dir(dirpath);
  QDir parent(dirpath);
  parent.cdUp();
  QString parentPath = parent.absolutePath();
  QString myPath = dir.absolutePath();
  QString myLeaf = parent.relativeFilePath(myPath);
  q.prepare("select id from folders where pathname==:p");
  q.bindValue(":p", parentPath);
  bool hasparent = false;
  quint64 parentid = 0;
  if (!q.exec()) {
    qDebug() << "FolderScanner::add could not search for parent";
    return;
  }
  if (q.next()) {
    hasparent = true;
    parentid = q.value(0).toULongLong();
  }

  q.prepare("begin transaction");
  if (!q.exec()) {
    qDebug() << "FolderScanner::add: Could not begin transaction";
    return;
  }


  if (hasparent) {
    q.prepare("insert into folders(parentfolder,leafname,pathname) "
              " values (:p,:l,:a)");
    q.bindValue(":p", parentid);
  } else {
    q.prepare("insert into folders(leafname,pathname) "
              " values (:l,:a)");
  }
  q.bindValue(":l", myLeaf);
  q.bindValue(":a", myPath);
  if (!q.exec()) {
    qDebug() << "FolderScanner::add: Could not insert folder";
    if (!q.exec("rollback transaction"))
      qDebug() << "  Could not roll back transaction";
    return;
  }
  quint64 myid = q.lastInsertId().toULongLong();
  qDebug() << "inserted " << myid << hasparent << parentid << myLeaf << myPath;

  if (hasparent) {
    q.prepare("insert into foldertree(descendant,ancestor) "
              " values (:d,:a)");
    q.bindValue(":d", myid);
    q.bindValue(":a", parentid);
    if (!q.exec()) {
      qDebug() << "FolderScanner::add: Could not add relationship";
      return;
      if (!q.exec("rollback transaction"))
        qDebug() << "  Could not roll back transaction";
    }
    q.prepare("insert into foldertree(descendant,ancestor) "
              " select ancestor, :d "
              " from foldertree where descendant==:a");
    q.bindValue(":d", myid);
    q.bindValue(":a", parentid);
    if (!q.exec()) {
      qDebug() << "FolderScanner::add: Could not add grand relationships";
      qDebug() << "   " << q.lastError().text();
      if (!q.exec("rollback transaction"))
        qDebug() << "  Could not roll back transaction";
      return;
    }
  }
  if (!q.exec("commit transaction")) {
    qDebug() << "FolderScanner::add: Could not commit transaction";
    return;
  }

  rescan(myid, internal);
}

void FolderScanner::run() {
  while (!stopsoon) {
    qDebug() << "FolderScanner: running";
    QSqlQuery q(db);
    q.prepare("select folder from folderstoscan limit 1");
    mutex.lock();
    if (!q.exec()) {
      mutex.unlock();
      qDebug() << "FolderScanner: Could not run select query";
      quit();
    }
    if (q.next()) {
      mutex.unlock();
      quint64 id = q.value(0).toULongLong();
      doscan(id);
      q.prepare("delete from folderstoscan where folder=:i");
      q.bindValue(":i", id);
      if (!q.exec()) {
        qDebug() << "FolderScanner: Could not remove from scanlist";
        quit();
      }
    } else {
      qDebug() << "FolderScanner: Waiting";
      waiter.wait(&mutex);
      mutex.unlock();
    }
  }    
}

void FolderScanner::doscan(quint64 folder) {
  QSqlQuery q(db);
  q.prepare("select pathname from folders where id==:i");
  q.bindValue(":i", folder);
  if (!q.exec()) {
    qDebug() << "FolderScanner::doscan: Failed to select pathname";
    quit();
  }
  if (!q.next()) {
    qDebug() << "FolderScanner::doscan: Folder not found: " << folder;
    return;
  }
  QDir dir(q.value(0).toString());
  qDebug() << "Scanning " << dir.absolutePath();
  QFileInfoList infos(dir.entryInfoList(QDir::Dirs | QDir::Files
                                        | QDir::NoDotAndDotDot));
  for (auto i: infos) {
    qDebug() << "entry" << i.fileName();
    if (i.isDir()) {
      add(i.absoluteFilePath(), true);
    } else {
      if (phscan) {
        phscan->add(folder, i.fileName());
      }
    }
  }
  qDebug() << "Done scanning " << dir.absolutePath();
}
