// Purge.cpp

#include "Purge.h"

void purgeRejected(PhotoDB *db, AutoCache *cache) {
  db->query("create table dupl as"
            " select photos.id as id, count(*) as n"
            " from versions inner join photos"
            " on versions.photo==photos.id group by photos.id");
  while (true) {
    QSqlQuery q = db->constQuery("select versions.id, dupl.id"
                                 " from versions inner join dupl"
                                 " on versions.photo==dupl.id"
                                 " where dupl.n>1 and versions.acceptreject<0"
                                 " limit 1");
    if (q.next()) {
    } else {
      break;
    }
  }
}
