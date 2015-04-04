// Selection.cpp

#include "Selection.h"
#include <QVariant>

Selection::Selection(PhotoDB const &db1, QObject *parent):
  QObject(parent), db(db1) {
  setObjectName("selection");
  db.query("create table if not exists M.selection ("
           " version integer unique on conflict ignore )");
           // " references versions(id)"
           // "   on delete cascade on update cascade)");
}

void Selection::add(quint64 vsn) {
  db.query("insert into M.selection values (:i)", vsn);
}

void Selection::addDateRange(QDateTime start, QDateTime inclusiveend) {
  db.query("insert into selection select versions.id from "
           " versions inner join photos on versions.photo=photos.id "
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
  db.query("insert into M.selection (select versions.id from versions)");
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
