// PurgeCache.cpp

#include "PurgeCache.h"
#include "PhotoDB.h"
#include "PDebug.h"
#include <QStringList>
#include <QDir>

namespace PurgeCache {
  void purge(PhotoDB &db, QString cachedir) {
    pDebug() << "Purging removed versions from cache";
    
    db.query(QString("attach '%1/cache.db' as C").arg(cachedir));
    QSqlQuery q = db.query("select max(dbno) from C.cache"
                           " where not C.cache.version in"
                           " (select id from versions)");
    if (!q.next()) {
      // nothing to do
      db.query("detach C");
      return;
    }
    int B = q.value(0).toInt();
    for (int b=1; b<=B; b++) 
      db.query(QString("attach '%1/blobs%2.db' as B%3")
               .arg(cachedir).arg(b).arg(b));

    q = db.query("select version,maxdim from C.cache"
                 " where C.cache.dbno==0"
                 " and not C.cache.version in (select id from versions)");
    while (q.next()) {
      quint64 vsn = q.value(0).toInt();
      int d = q.value(1).toInt();
      QList<int> bits;
      while (vsn>0) {
        bits << vsn % 100;
        vsn /= 100;
      }
      QString leaf(QString("%1-%2.jpg").arg(bits.takeFirst()).arg(d));
      QStringList pathlist;
      for (auto b: bits)
        pathlist.push_front(QString("%1").arg(b));
      QDir dir(cachedir + "/thumbs/" + pathlist.join("/"));
      dir.remove(leaf);
      while (!pathlist.isEmpty()) {
        dir.cdUp();
        if (!dir.rmdir(pathlist.takeLast()))
          break;
      }
    }

    db.query("delete from C.cache where"
             " not C.cache.version in (select id from versions)");

    for (int b=1; b<=B; b++)
      db.query(QString("delete from B%1.blobs where"
                       " not B%2.blobs.cacheid in (select id from C.cache)")
               .arg(b).arg(b));

    for (int b=1; b<=B; b++)
      db.query(QString("detach B%1").arg(b));

    db.query("detach C");
  }
};
