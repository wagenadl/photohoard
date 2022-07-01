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
  attach();
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
  attach();
}  

void BasicCache::attach() {
  DBWriteLock lock(&db);
  QString q1 = "attach '" + root.absolutePath() + "/blobs%1.db' as B%2";
  QString q2 = "create table if not exists B%1.blobs ("
    " cacheid integer primary key on conflict replace,"
    " bits blob )";
  for (int k=1; k<=stdsizes.size(); k++) {
    db.query(q1.arg(k).arg(k));
    db.query(q2.arg(k));
  }
}

BasicCache::~BasicCache() {
  if (db.isOpen()) {
    CRASH("BasicCache deleted while open");
    close();
  }
}
 
void BasicCache::close() {
  { DBWriteLock lock(&db);
    for (int k=1; k<=stdsizes.size(); k++) 
      db.query(QString("detach B%1").arg(k));
  }
  db.close();
}

void BasicCache::readConfig() {
  DBReadLock lock(&db);
  QSqlQuery q = db.constQuery("select bytes from memthresh");
  if (q.next()) {
    memthresh = q.value(0).toInt();
  } else {
    memthresh = 200000;
    CRASH("Could not read memory threshold from db");
  }

  QList<int> sizes;
  q = db.constQuery("select maxdim from sizes");
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
  QSqlQuery q(db.constQuery("select id, maxdim, dbno from cache"
		       " where version==:a and outdated>0", vsn));
  while (q.next()) {
    quint64 id = q.value(0).toULongLong();
    int d = q.value(1).toInt();
    int k = q.value(2).toInt();
    if (k==0)
      QFile(constructFilename(vsn, d)).remove();
    else
      db.query(QString("delete from B%1.blobs where cacheid=:a").arg(k), id);
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
  int k = 0;
  if (buf.data().size() < memthresh) {
    k = 1;
    for (int l=1; l<stdsizes.size(); l++)
      if (s.exceeds(stdsizes[l]))
        k++;
  }

  int d = s.maxDim();


  QSqlQuery q = db.constQuery("select id, dbno from cache"
			 " where version==:a and maxdim==:b",
			 vsn, d);
  if (q.next()) {
    // preexist
    quint64 cacheid = q.value(0).toULongLong();
    int oldk = q.value(1).toInt();

    if (oldk && k!=oldk) 
      db.query(QString("delete from B%1.blobs where cacheid==:a").arg(oldk),
               cacheid);
    else if (!oldk && k)
      QFile(constructFilename(vsn, d)).remove();
    
    db.query("update cache set dbno=:a, width=:b, height=:c, outdated=:d"
             " where version==:e and maxdim==:f",
	     k, s.width(), s.height(),
	     instantlyOutdated ? 1 : 0,
	     vsn, d);

    if (k) {
      db.query(QString("insert into B%1.blobs (cacheid, bits) values(:a,:b)")
               .arg(k), cacheid, buf.data());
    } else {
      QFile f(constructFilename(vsn, d));
      if (f.open(QFile::WriteOnly)) 
	f.write(buf.data());
      else
	CRASH("Cannot write");
    }
  } else {
    // new
    quint64 cacheid
      = db.query("insert into cache"
		 " (version, maxdim, width, height, outdated, dbno)"
		 " values (:a, :b, :c, :d, :e, :f)",
		 vsn, d, s.width(), s.height(), instantlyOutdated?1:0, k)
      .lastInsertId().toULongLong();

    if (k) {
      db.query(QString("insert into B%1.blobs (cacheid, bits) values (:a, :b)")
               .arg(k), cacheid, buf.data());
    } else {
      QFile f(constructFilename(vsn, d));
      if (f.open(QFile::WriteOnly)) 
	f.write(buf.data());
      else
	CRASH("Cannot write");
    }
  }
}

void BasicCache::remove(quint64 vsn) {
  // should be called within a transaction
  QSqlQuery q(db.constQuery("select id, maxdim, dbno from cache"
		       " where version==:a", vsn));
  while (q.next()) {
    quint64 cacheid = q.value(0).toULongLong();
    int d = q.value(1).toInt();
    int k = q.value(2).toInt();
    if (k)
      db.query(QString("delete from B%1.blobs where cacheid==:a").arg(k),
               cacheid);
    else
      QFile(constructFilename(vsn, d)).remove();
  }
  db.query("delete from cache where version==:a", vsn);
}

Image16 BasicCache::get(quint64 vsn, PSize s, bool *outdated_return) {
  quint64 cacheid;
  int k;
  bool od;
  { DBReadLock lock(&db);
    QSqlQuery q = db.constQuery("select id, dbno, outdated from cache"
                                " where version==:a and maxdim==:b limit 1",
                                vsn, s.maxDim());
    ASSERT(q.next());
    cacheid = q.value(0).toInt();
    k = q.value(1).toInt();
    od = q.value(2).toInt()>0;
  }
  if (outdated_return)
    *outdated_return = od;
  if (k) {
    QByteArray bits;
    { DBReadLock lock(&db);
      bits = db.simpleQuery(QString("select bits from B%1.blobs"
                                    " where cacheid==:a").arg(k),
                            cacheid)
        .toByteArray();
    }
    QBuffer buf(&bits);
    QImageReader reader(&buf, "jpeg");
    return reader.read();
  } else {
    QString fn(constructFilename(vsn, s.maxDim()));
    if (QFile(fn).exists()) 
      return Image16(fn);
    CRASH("Missing file " + fn + " from cache"); // could tolerate
    return Image16();
  }
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

QString BasicCache::constructFilename(quint64 vsn, int d) {
  QList<int> bits;
  while (vsn>0) {
    bits << vsn % 100;
    vsn /= 100;
  }
  QString leaf(QString("%1-%2.jpg").arg(bits.takeFirst()).arg(d));
  QStringList pathlist;
  for (auto b: bits)
    pathlist.push_front(QString("%1").arg(b));
  pathlist.push_front("thumbs");
  QString path = pathlist.join("/");
  root.mkpath(path);
  return root.absoluteFilePath(path + "/" + leaf);
}

void BasicCache::markOutdated(quint64 vsn) {
  // must be called inside a transaction
  db.query("update cache set outdated=1 where version==:a", vsn);
}

PSize BasicCache::maxSize() const {
  return stdsizes[0];
}
