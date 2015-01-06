// Selection.cpp

#include "Selection.h"
#include <QVariant>

Selection::Selection(Database const &db1, QObject *parent):
  QObject(parent), db(db1) {
  setObjectName("selection");
  db.query("create table if not exists M.selection ("
           " version integer unique on conflict ignore "
           " references versions(id)"
           "   on delete cascade on update cascade)");
}

void Selection::add(quint64 vsn) {
  db.query("insert into M.selection values (:i)", vsn);
}

void Selection::add(QDateTime start, QDateTime inclusiveend) {
  db.query("insert into M.selection (select versions.id from "
           " versions inner join photos on versions.photo=photos.id "
           " where photos.capturedata>=:a and photos.capturedate<=:b",
           start, inclusiveend);
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




  
