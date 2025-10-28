// PurgeCache.cpp

#include "PurgeCache.h"
#include "PhotoDB.h"
#include "PDebug.h"
#include <QStringList>
#include <QDir>

#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/slice.h"


namespace PurgeCache {
  void purge(PhotoDB &db, QString cachedir) {
    DBWriteLock lock(&db);

    db.query("attach :a as C", cachedir + "/cache.db");
    QSqlQuery q = db.constQuery("select 1 from C.cache"
                           " where not C.cache.version in"
                           " (select id from versions)");
    if (!q.next()) {
      // nothing to do
      db.query("detach C");
      return;
    }

    rocksdb::Options options;
    rocksdb::DB *rdb;
    rocksdb::Status s = rocksdb::DB::Open(options,
                                          std::string(cachedir.toUtf8().data()),
                                          &rdb);
    assert(s.ok());
  
    q = db.constQuery("select id from C.cache"
                      " where not C.cache.version in"
                      " (select id from versions)");
    while (q.next()) {
      quint64 id = q.value(0).toULongLong();
      rdb->Delete(rocksdb::WriteOptions(),
                  rocksdb::Slice((char const *)&id, sizeof(id)));
    }
    delete rdb;

    db.query("delete from C.cache"
             " where not C.cache.version in (select id from versions)");
    
    db.query("detach C");
  }
};
