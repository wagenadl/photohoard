// BasicCache.cpp

#include "BasicCache.h"
#include "PDebug.h"
#include "Database.h"
#include <QBuffer>
#include <QImageWriter>
#include <QImageReader>
#include <system_error>
#include "SqlFile.h"
#include <algorithm>
#include "SessionDB.h"

#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/slice.h"


class RocksDB: public QSharedData {
public:
  rocksdb::DB *db;
  RocksDB() { db = 0; }
  ~RocksDB() { delete db; }
};

BasicCache::BasicCache() {
}

void BasicCache::open(QString rootdir) {
  if (isOpen())
    close();
  
  root.setPath(rootdir);
  db.open(rootdir + "/cache.db");
  readConfig();
  { DBWriteLock lock(&db);
    db.query("pragma synchronous = 0");
  }

  // ROCKSDB
  rocksdb::Options options;
  // Optimize RocksDB. This is the easiest way to get RocksDB to perform well
  options.IncreaseParallelism();
  options.OptimizeLevelStyleCompaction();
  // create the DB if it's not already present
  options.create_if_missing = true;

  // open DB
  RocksDB *rdb1 = new RocksDB;
  rocksdb::Status s = rocksdb::DB::Open(options,
                                        std::string(rootdir.toUtf8().data()),
                                        &rdb1->db);
  assert(s.ok());
  rdb = rdb1;
}

void BasicCache::clone(BasicCache const &src) {
  if (isOpen())
    close();

  root = src.root;
  db.clone(src.db);
  readConfig();

  { DBWriteLock lock(&db);
    db.query("pragma synchronous = 0");
  }

  rdb = src.rdb;
}  


BasicCache::~BasicCache() {
  if (db.isOpen()) {
    CRASH("BasicCache deleted while open");
    close();
  }
}
 
void BasicCache::close() {
  db.close();
}

void BasicCache::readConfig() {
  DBReadLock lock(&db);
  QList<int> sizes;
  QSqlQuery q = db.constQuery("select maxdim from sizes");
  while (q.next())
    sizes << q.value(0).toInt();
  
  std::sort(sizes.begin(), sizes.end(), [](int a, int b) { return a > b; });
  //  qDebug() << "sizes" << sizes;

  stdsizes.clear();
  for (int s: sizes)
    stdsizes << PSize::square(s);
}
  
void BasicCache::create(QString rootdir) {
  qDebug() << "creating cache" << rootdir;
  QDir root(rootdir);
  
  ASSERT(!root.exists());


  if (!QDir("/").mkpath(root.absolutePath()))
    CRASH("BasicCache::create: Could not create directory: "
          + root.absolutePath());

  Database db;
  db.open(rootdir + "/cache.db");
  Transaction t(&db);
  SqlFile sql(":/setupcache.sql");
  for (auto c: sql) 
    db.query(c);
  t.commit();
  db.close();
}

void BasicCache::add(quint64 vsn, Image16 img, bool instantlyOutdated) {
  // should be called within a transaction
  PSize s0 = maxSize();
  bool done = false;

  if (img.size().isContainedIn(s0)) {
    // cache image directly: it is no larger than our largest desired size
    addToCache(vsn, img, instantlyOutdated);
    done = instantlyOutdated;
  }
  if (!done) {
    for (auto s: stdsizes) {
      if (img.size().exceeds(s)) {
        img = img.scaledToFitSnuglyIn(s);
        addToCache(vsn, img, instantlyOutdated);
        if (instantlyOutdated)
          break;
      }
    }
  }
  if (!instantlyOutdated)
    dropOutdatedFromCache(vsn);
}

void BasicCache::dropOutdatedFromCache(quint64 vsn) {
  // should be called within a transaction
  QSqlQuery q(db.constQuery("select id from cache"
		       " where version==:a and outdated>0", vsn));
  while (q.next()) {
    quint64 id = q.value(0).toULongLong();
    rdb.data()->db->Delete(rocksdb::WriteOptions(),
                       rocksdb::Slice((char const *)&id, sizeof(id)));
  }

  db.query("delete from cache where version==:a and outdated>0", vsn);
}

void BasicCache::addToCache(quint64 vsn, Image16 const &img,
                            bool instantlyOutdated) {
  // should be called within a transaction
  QBuffer buf;
  QImageWriter writer(&buf, "jpeg");
  writer.write(img.toQImage());

  PSize s = img.size();
  Q_ASSERT(!s.exceeds(maxSize()));
  int d = s.maxDim();


  QSqlQuery q = db.constQuery("select id from cache"
			 " where version==:a and maxdim==:b",
			 vsn, d);
  if (q.next()) {
    // preexist
    quint64 cacheid = q.value(0).toULongLong();
    rdb.data()->db->Put(rocksdb::WriteOptions(),
                    rocksdb::Slice((char const *)&cacheid, sizeof(cacheid)),
                    rocksdb::Slice(buf.data().constData(), buf.data().size()));
    
    db.query("update cache set dbno=:a, width=:b, height=:c, outdated=:d"
             " where version==:e and maxdim==:f",
	     0, s.width(), s.height(),
	     instantlyOutdated ? 1 : 0,
	     vsn, d);
  } else {
    // new
    quint64 cacheid
      = db.query("insert into cache"
		 " (version, maxdim, width, height, outdated, dbno)"
		 " values (:a, :b, :c, :d, :e, :f)",
		 vsn, d, s.width(), s.height(), instantlyOutdated?1:0, 0)
      .lastInsertId().toULongLong();

    rdb.data()->db->Put(rocksdb::WriteOptions(),
                    rocksdb::Slice((char const *)&cacheid, sizeof(cacheid)),
                    rocksdb::Slice(buf.data().constData(), buf.data().size()));
  }
}

void BasicCache::remove(quint64 vsn) {
  // should be called within a transaction
  QSqlQuery q(db.constQuery("select id from cache"
		       " where version==:a", vsn));
  while (q.next()) {
    quint64 cacheid = q.value(0).toULongLong();
    rdb.data()->db->Delete(rocksdb::WriteOptions(),
                       rocksdb::Slice((char const *)&cacheid, sizeof(cacheid)));
  }
  db.query("delete from cache where version==:a", vsn);
}

Image16 BasicCache::get(quint64 vsn, PSize s, bool *outdated_return) {
  quint64 cacheid;
  bool od;
  { DBReadLock lock(&db);
    QSqlQuery q = db.constQuery("select id, dbno, outdated from cache"
                                " where version==:a and maxdim==:b limit 1",
                                vsn, s.maxDim());
    ASSERT(q.next());
    // can we not recover from this somehow?
    cacheid = q.value(0).toInt();
    od = q.value(2).toInt()>0;
  }
  if (outdated_return)
    *outdated_return = od;
  rocksdb::PinnableSlice val;
  rdb.data()->db->Get(rocksdb::ReadOptions(),
                      rdb.data()->db->DefaultColumnFamily(),
                  rocksdb::Slice((char const *)&cacheid, sizeof(cacheid)),
                  &val);
  // check for errors?
  QByteArray bits(val.data(), val.size());
  QBuffer buf(&bits);
  QImageReader reader(&buf, "jpeg");
  return reader.read();
}

PSize BasicCache::bestSize(quint64 vsn, PSize desired) {
  DBReadLock lock(&db);
  QSqlQuery q(db.constQuery("select width, height, outdated from cache"
		       " where version==:a", vsn));
  PSize sbest;
  bool gotbig = false;
  bool outdated = true;
  while (q.next()) {
    PSize s(q.value(0).toInt(), q.value(1).toInt());
    bool od = q.value(2).toInt()>0;
    if (od && !outdated) {
      // If I have an up-to-date version, never replace w/ outdated.
    } else if (outdated && !od) {
      // Always replace outdated with up-to-date.
      sbest = s;
      outdated = false;
    } else if (s.isLargeEnoughFor(desired)) {
      // Nice and big, but perhaps too big
      if (s<sbest || !gotbig) {
	sbest = s;
	outdated = od;
	gotbig = true;
      }
    } else {
      // Small, but perhaps better than what we have
      if (sbest<s && !gotbig) {
	sbest = s;
	outdated = od;
      }
    }
  }
  return sbest;  
}

bool BasicCache::isOutdated(quint64 vsn) {
  return db.simpleQuery("select count(*) from cache where version==:a"
			" and outdated==1", vsn).toInt() > 0;
}

bool BasicCache::contains(quint64 vsn, bool outdatedOK) {
  QString query = "select count(*) from cache where version==:a";
  if (!outdatedOK)
    query += " and outdated==0";
  query += " limit 1";
  return db.simpleQuery(query, vsn).toInt()>0;
}

QList<PSize> BasicCache::sizes(quint64 vsn, bool outdatedOK) {
  QString query = "select width, height from cache where version==:v";
  if (!outdatedOK)
    query += " and outdated==0";
  query += " order by maxdim";
  QSqlQuery q(db.constQuery(query, vsn));
  
  QList<PSize> lst;
  while (q.next()) {
    int w = q.value(0).toInt();
    int h = q.value(1).toInt();
    lst << PSize(w, h);
  }

  return lst;
}

void BasicCache::markOutdated(quint64 vsn) {
  // must be called inside a transaction
  db.query("update cache set outdated=1 where version==:a", vsn);
}

PSize BasicCache::maxSize() const {
  return stdsizes[0];
}
