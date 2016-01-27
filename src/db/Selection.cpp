// Selection.cpp

#include "Selection.h"
#include <QVariant>
#include "PDebug.h"

Selection::Selection(PhotoDB *db): db(db) {
}

void Selection::add(quint64 vsn) {
  Untransaction t(db);
  db->query("insert into M.selection values (:i)", vsn);
}

void Selection::addDateRange(QDateTime start, QDateTime inclusiveend) {
  Untransaction t(db);
  db->query("insert into selection select version from "
           " filter inner join photos on filter.photo=photos.id "
           " where photos.capturedate>=:a and photos.capturedate<=:b",
           start, inclusiveend);
}

void Selection::addDateRange(QDateTime start, Strip::TimeScale scl) {
  addDateRange(start, Strip::endFor(start, scl).addMSecs(-1));
}

void Selection::dropDateRange(QDateTime start, QDateTime inclusiveend) {
  Untransaction t(db);
  db->query("delete from selection where version in (select version from "
           " filter inner join photos on filter.photo=photos.id "
           " where photos.capturedate>=:a and photos.capturedate<=:b)",
           start, inclusiveend);
}

void Selection::dropDateRange(QDateTime start, Strip::TimeScale scl) {
  dropDateRange(start, Strip::endFor(start, scl).addMSecs(-1));
}
  
void Selection::remove(quint64 vsn) {
  Untransaction t(db);
  db->query("delete from M.selection where version==:i", vsn);
}
  
void Selection::clear() {
  Untransaction t(db);
  db->query("delete from M.selection");
}

void Selection::selectAll() {
  Untransaction t(db);
  db->query("insert into M.selection select version from filter");
}

bool Selection::contains(quint64 vsn) {
  return db->simpleQuery("select count(*) from M.selection "
                        " where version==:v limit 1",
                        vsn).toInt() > 0;
}

int Selection::count() {
  return db->simpleQuery("select count(*) from M.selection").toInt();
}

QSet<quint64> Selection::current() {
  QSqlQuery q = db->query("select version from selection");
  QSet<quint64> vv;
  while (q.next())
    vv << q.value(0).toULongLong();
  return vv;
}

void Selection::addStartOfFolder(quint64 folder, QDateTime endAt) {
  Untransaction t(db);
  db->query("insert into selection select version from filter"
           " inner join photos on filter.photo==photos.id"
           " where photos.folder==:a and photos.capturedate<=:b",
           folder, endAt);
}

void Selection::addRestOfFolder(quint64 folder, QDateTime startAt) {
  Untransaction t(db);
  db->query("insert into selection select version from filter"
           " inner join photos on filter.photo==photos.id"
           " where photos.folder==:a and photos.capturedate>=:b",
           folder, startAt);
}

void Selection::addFoldersBetween(quint64 fid1, quint64 fid2) {
  QString path1 = db->folder(fid1);
  QString path2 = db->folder(fid2);
  Untransaction t(db);
  db->query("insert into selection select version from filter"
           " inner join photos on filter.photo==photos.id"
           " inner join folders on photos.folder==folders.id"
           " where folders.pathname>:a and folders.pathname<:b",
           path1, path2);
}
  
int Selection::countInDateRange(QDateTime t0, QDateTime t1) const {
  return db->simpleQuery("select count(*) from selection inner join filter"
                        " on selection.version==filter.version"
                        " inner join photos on filter.photo==photos.id"
                        " where photos.capturedate>=:a"
                        " and photos.capturedate<:b", t0, t1).toInt();
}

int Selection::countInFolder(QString folder) const {
  quint64 id = db->findFolder(folder);
  if (id)
    return db->simpleQuery("select count(*) from selection inner join filter"
                          " on selection.version==filter.version"
                          " inner join photos on filter.photo=photos.id"
                          " where photos.folder==:a", id).toInt();
  else
    return 0;
}

int Selection::countInTree(QString folder) const {
  int nsub
    = db->simpleQuery("select count(*) from selection"
                     " inner join filter on selection.version==filter.version"
                     " inner join photos on filter.photo=photos.id"
                     " inner join folders on photos.folder==folders.id"
		     " where folders.id in"
		     " (select id from folders where pathname like :a)",
		     folder+"/%").toInt();
  return nsub + countInFolder(folder);
}

void Selection::addInFolder(QString folder) {
  quint64 id = db->findFolder(folder);
  if (id) {
    Untransaction t(db);
    db->query("insert into selection select version from filter"
             " inner join photos on filter.photo=photos.id"
             " where photos.folder==:a", id);
  }
}

void Selection::addInTree(QString folder) {
  addInFolder(folder);
  Untransaction t(db);
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
    Untransaction t(db);
    db->query("delete from selection where version in"
             " (select version from filter"
             " inner join photos on filter.photo=photos.id"
             " where photos.folder==:a)", id);
  }
}

void Selection::dropInTree(QString folder) {
  dropInFolder(folder);
  Untransaction t(db);
  db->query("delete from selection select version from filter"
           " inner join photos on filter.photo=photos.id"
           " inner join folders on photos.folder==folders.id"
           " where folders.pathname like :a", folder+"/%");
}

  
