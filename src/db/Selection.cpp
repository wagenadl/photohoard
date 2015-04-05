// Selection.cpp

#include "Selection.h"
#include <QVariant>

Selection::Selection(PhotoDB const &db1): db(db1) {
}

void Selection::add(quint64 vsn) {
  db.query("insert into M.selection values (:i)", vsn);
}

void Selection::addDateRange(QDateTime start, QDateTime inclusiveend) {
  db.query("insert into selection select version from "
           " filter inner join photos on filter.photo=photos.id "
           " where photos.capturedate>=:a and photos.capturedate<=:b",
           start, inclusiveend);
}

void Selection::addDateRange(QDateTime start, Strip::TimeScale scl) {
  addDateRange(start, Strip::endFor(start, scl).addMSecs(-1));
}
  
void Selection::remove(quint64 vsn) {
  db.query("delete from M.selection where version==:i", vsn);
}
  
void Selection::clear() {
  db.query("delete from M.selection");
}

void Selection::selectAll() {
  db.query("insert into M.selection (select version from filter)");
}

bool Selection::contains(quint64 vsn) {
  return db.simpleQuery("select count(*) from M.selection "
                        " where version==:v limit 1",
                        vsn).toInt() > 0;
}

int Selection::count() {
  return db.simpleQuery("select count(*) from M.selection").toInt();
}

QSet<quint64> Selection::current() {
  QSqlQuery q = db.query("select version from selection");
  QSet<quint64> vv;
  while (q.next())
    vv << q.value(0).toULongLong();
  return vv;
}

void Selection::addStartOfFolder(quint64 folder, QDateTime endAt) {
  db.query("insert into selection select version from filter"
           " inner join photos on filter.photo==photos.id"
           " where photos.folder==:a and photos.capturedate<=:b",
           folder, endAt);
}

void Selection::addRestOfFolder(quint64 folder, QDateTime startAt) {
  db.query("insert into selection select version from filter"
           " inner join photos on filter.photo==photos.id"
           " where photos.folder==:a and photos.capturedate>=:b",
           folder, startAt);
}

void Selection::addFoldersBetween(quint64 fid1, quint64 fid2) {
  QString path1 = db.folder(fid1);
  QString path2 = db.folder(fid2);
  db.query("insert into selection select version from filter"
           " inner join photos on filter.photo==photos.id"
           " inner join folders on photos.folder==folders.id"
           " where folders.pathname>:a and folders.pathname<:b",
           path1, path2);
}
  
