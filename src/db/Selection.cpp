// Selection.cpp

#include "Selection.h"
#include <QVariant>
#include "PDebug.h"

Selection::Selection(PhotoDB *db): db(db) {
}

void Selection::add(quint64 vsn) {
  DBWriteLock lock(db);
  db->query("insert into selection values (:a)", vsn);
}

void Selection::addDateRange(QDateTime start, QDateTime inclusiveend) {
  DBWriteLock lock(db);
  db->query("insert into selection select version from "
           " filter inner join photos on filter.photo=photos.id "
           " where photos.capturedate>=:a and photos.capturedate<=:b",
           start.toString(Qt::ISODate), inclusiveend.toString(Qt::ISODate));
}

void Selection::addDateRange(QDateTime start, Strip::TimeScale scl) {
  addDateRange(start, Strip::endFor(start, scl).addMSecs(-1));
}

void Selection::dropDateRange(QDateTime start, QDateTime inclusiveend) {
  DBWriteLock lock(db);
  db->query("delete from selection where version in (select version from "
           " filter inner join photos on filter.photo=photos.id "
           " where photos.capturedate>=:a and photos.capturedate<=:b)",
            start.toString(Qt::ISODate), inclusiveend.toString(Qt::ISODate));
}

void Selection::dropDateRange(QDateTime start, Strip::TimeScale scl) {
  dropDateRange(start, Strip::endFor(start, scl).addMSecs(-1));
}
  
void Selection::remove(quint64 vsn) {
  DBWriteLock lock(db);
  db->query("delete from selection where version==:a", vsn);
}
  
void Selection::clear() {
  DBWriteLock lock(db);
  db->query("delete from selection");
}

void Selection::selectAll() {
  DBWriteLock lock(db);
  db->query("insert into selection select version from filter");
}

bool Selection::contains(quint64 vsn) {
  DBReadLock lock(db);
  return db->simpleQuery("select count(*) from selection "
                        " where version==:v limit 1",
                        vsn).toInt() > 0;
}

int Selection::count() {
  DBReadLock lock(db);
  return db->simpleQuery("select count(*) from selection").toInt();
}

QSet<quint64> Selection::current() {
  DBReadLock lock(db);
  QSqlQuery q = db->constQuery("select version from selection");
  QSet<quint64> vv;
  while (q.next())
    vv << q.value(0).toULongLong();
  return vv;
}

void Selection::addStartOfFolder(quint64 folder, QDateTime endAt) {
  DBWriteLock lock(db);
  db->query("insert into selection select version from filter"
           " inner join photos on filter.photo==photos.id"
           " where photos.folder==:a and photos.capturedate<=:b",
           folder, endAt.toString(Qt::ISODate));
}

void Selection::addRestOfFolder(quint64 folder, QDateTime startAt) {
  DBWriteLock lock(db);
  db->query("insert into selection select version from filter"
           " inner join photos on filter.photo==photos.id"
           " where photos.folder==:a and photos.capturedate>=:b",
           folder, startAt.toString(Qt::ISODate));
}

void Selection::addFoldersBetween(quint64 fid1, quint64 fid2) {
  QString path1 = db->folder(fid1);
  QString path2 = db->folder(fid2);
  DBWriteLock lock(db);
  db->query("insert into selection select version from filter"
           " inner join photos on filter.photo==photos.id"
           " inner join folders on photos.folder==folders.id"
           " where folders.pathname>:a and folders.pathname<:b",
           path1, path2);
}
  
int Selection::countInDateRange(QDateTime t0, QDateTime t1) const {
  DBReadLock lock(db);
  return db->simpleQuery("select count(*) from selection inner join filter"
                        " on selection.version==filter.version"
                        " inner join photos on filter.photo==photos.id"
                        " where photos.capturedate>=:a"
                        " and photos.capturedate<:b",
                         t0.toString(Qt::ISODate),
                         t1.toString(Qt::ISODate)).toInt();
}

int Selection::countInFolder(QString folder) const {
  quint64 id = db->findFolder(folder);
  if (!id)
    return 0;
  DBReadLock lock(db);
  return db->simpleQuery("select count(*) from selection inner join filter"
                         " on selection.version==filter.version"
                         " inner join photos on filter.photo=photos.id"
                         " where photos.folder==:a", id).toInt();
}

int Selection::countInTree(QString folder) const {
  int nsub = countInFolder(folder);
  DBReadLock lock(db);
  return nsub
    + db->simpleQuery("select count(*) from selection"
                      " inner join filter on selection.version==filter.version"
                      " inner join photos on filter.photo=photos.id"
                      " inner join folders on photos.folder==folders.id"
                      " where folders.id in"
                      " (select id from folders where pathname like :a)",
                       folder+"/%").toInt();
}

void Selection::addInFolder(QString folder) {
  quint64 id = db->findFolder(folder);
  if (id) {
    DBWriteLock lock(db);
    db->query("insert into selection select version from filter"
             " inner join photos on filter.photo=photos.id"
             " where photos.folder==:a", id);
  }
}

void Selection::addInTree(QString folder) {
  addInFolder(folder);
  DBWriteLock lock(db);
  db->query("insert into selection select version from filter"
           " inner join photos on filter.photo=photos.id"
           " inner join folders on photos.folder==folders.id"
	   " where folders.id in"
	   " (select id from folders where pathname like :a)",
	   folder+"/%");
}

void Selection::dropInFolder(QString folder) {
  quint64 id = db->findFolder(folder);
  if (id) {
    DBWriteLock lock(db);
    db->query("delete from selection where version in"
             " (select version from filter"
             " inner join photos on filter.photo=photos.id"
             " where photos.folder==:a)", id);
  }
}

void Selection::dropInTree(QString folder) {
  dropInFolder(folder);
  DBWriteLock lock(db);
  db->query("delete from selection select version from filter"
           " inner join photos on filter.photo=photos.id"
           " inner join folders on photos.folder==folders.id"
           " where folders.pathname like :a", folder+"/%");
}

  
