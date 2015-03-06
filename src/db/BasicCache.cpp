// BasicCache.cpp

#include "BasicCache.h"
#include <QDebug>
#include "Database.h"
#include <QBuffer>
#include <QImageWriter>
#include <QImageReader>
#include <system_error>
#include "SqlFile.h"

BasicCache::BasicCache(QString rootdir, QObject *parent):
  QObject(parent), root(rootdir), db(rootdir + "/cache.db") {
  setObjectName("BasicCache");
  qDebug() << "BasicCache constructor 1";
  readConfig();
  attach();
  qDebug() << "BasicCache constructed 1";
}

BasicCache::BasicCache(QDir root, Database const &db, QObject *parent):
  QObject(parent), root(root), db(db) {
  setObjectName("BasicCache");
  qDebug() << "BasicCache constructor 2";
  readConfig();
  attach();
  qDebug() << "BasicCache constructed 2";
}

void BasicCache::attach() {
  QString q1 = "attach '" + root.absolutePath() + "/blobs%1.db' as B%2";
  QString q2 = "create table if not exists B%1.blobs ("
    " cacheid integer primary key on conflict replace,"
    " bits blob )";
  for (int k=1; k<=stdsizes.size(); k++) {
    db.query(q1.arg(k).arg(k));
    db.query(q2.arg(k));
  }
  db.query("pragma synchronous = 0");
}

BasicCache::~BasicCache() {
  for (int k=1; k<=stdsizes.size(); k++) 
    db.query(QString("detach B%1").arg(k));
  QSqlDatabase::removeDatabase(root.absolutePath());
}

void BasicCache::readConfig() {
  QSqlQuery q(*db);
  if (!q.exec("select bytes from memthresh"))
    throw q;
  if (q.next()) {
    memthresh = q.value(0).toInt();
  } else {
    memthresh = 200000;
    qDebug() << "Could not read memory threshold from db";
  }

  if (!q.exec("select maxdim from sizes"))
    throw q;
  while (q.next())
    stdsizes << q.value(0).toInt();
  qSort(stdsizes.begin(), stdsizes.end(), qGreater<int>());
}
  
BasicCache *BasicCache::create(QString rootdir) {
  qDebug() << "BasicCache::create";
  QDir root(rootdir);
  
  if (root.exists()) 
    throw std::system_error(std::make_error_code(std::errc::file_exists));


  if (!root.mkdir(root.absolutePath())) {
    qDebug() << "BasicCache::create: Could not create directory.";
    throw std::system_error(std::error_code());
  }

  try {
    Database db(rootdir + "/cache.db");
    SqlFile sql(":/setupcache.sql");
    QSqlQuery q(*db);
    db.beginAndLock();
    for (auto c: sql) {
      if (!q.exec(c)) {
	qDebug() << "BasicCache: Could not setup: " << q.lastError().text();
	qDebug() << "  at " << c;
	db.rollbackAndUnlock();
	throw q;
      }
    }
    db.commitAndUnlock();
    qDebug() << "BasicCache created";
    return new BasicCache(root, db);
  } catch (...) {
    qDebug() << "BasicCache caught error while creating. Failure.";
    root.remove("cache.db");
    root.rmdir(root.absolutePath());
    throw;
  }
}

int BasicCache::maxdim(QSize const &s) {
  int w = s.width();
  int h = s.height();
  return w>h ? w : h;
}

Image16 BasicCache::sufficientSize(Image16 const &img) {
  int d = img.size().maxdim();
  int s0 = stdsizes[0];
  if (d>s0)
    return img.scaled(PSize(s0, s0), Qt::KeepAspectRatio);
  else
    return img;
}

void BasicCache::add(quint64 vsn, Image16 img, bool instantlyOutdated) {
  int d = maxdim(img.size());
  bool got = false;
  if (d<=stdsizes[0]) {
    // cache image directly: it is smaller than our largest desired size
    addToCache(vsn, img, instantlyOutdated);
    got = true;
  }
  if (!instantlyOutdated || !got) {
    for (auto s: stdsizes) {
      if (s<d) {
        img = img.scaled(QSize(s,s), Qt::KeepAspectRatio);
        addToCache(vsn, img);
        if (instantlyOutdated)
          break;
      }
    }
    if (!instantlyOutdated)
      dropOutdatedFromCache(vsn);
  }
}

void BasicCache::dropOutdatedFromCache(quint64 vsn) {
  QSqlQuery q(db.query("select id, maxdim, dbno from cache"
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
  QBuffer buf;
  QImageWriter writer(&buf, "jpeg");
  writer.write(img.toQImage());

  int d = maxdim(img.size());
  int k = 0;
  if (buf.data().size() < memthresh) {
    k = 1;
    for (int l=1; l<stdsizes.size(); l++)
      if (d>stdsizes[l])
        k++;
  }

  QSqlQuery q = db.query("select id, dbno from cache"
			 " where version==:a and maxdim==:b", vsn, d);
  if (q.next()) {
    // preexist
    quint64 cacheid = q.value(0).toULongLong();
    int oldk = q.value(1).toInt();
    qDebug() << "addToCache UPDATE" << vsn << d << cacheid << oldk << k;
    if (oldk && k!=oldk) 
      db.query(QString("delete from B%1.blobs where cacheid==:a").arg(oldk),
               cacheid);
    else if (!oldk && k)
      QFile(constructFilename(vsn, d)).remove();
    
    db.query("update cache set dbno=:a"
             " where version==:b and maxdim==:c", k, vsn, d);

    if (k) {
      db.query(QString("insert into B%1.blobs (cacheid, bits) values(:a,:b)")
               .arg(k), cacheid, buf.data());
    } else {
      QFile f(constructFilename(vsn, d));
      if (f.open(QFile::WriteOnly)) 
	f.write(buf.data());
      else
	throw std::system_error(std::error_code());
    }
  } else {
    // new
    quint64 cacheid = db.query("insert into cache"
                               " (version, maxdim, outdated, dbno)"
                               " values (:a, :b, :c, :d)",
                               vsn, d, instantlyOutdated?1:0, k)
      .lastInsertId().toULongLong();
    qDebug() << "addToCache NEW" << vsn << d << cacheid << k;
    if (k) {
      db.query(QString("insert into B%1.blobs (cacheid, bits) values (:a, :b)")
               .arg(k), cacheid, buf.data());
    } else {
      QFile f(constructFilename(vsn, d));
      if (f.open(QFile::WriteOnly)) 
	f.write(buf.data());
      else
	throw std::system_error(std::error_code());
    }
  }
}

void BasicCache::remove(quint64 vsn) {
  QSqlQuery q(db.query("select id, maxdim, dbno from cache"
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

Image16 BasicCache::get(quint64 vsn, int maxdim, bool *outdated_return) {
  QSqlQuery q = db.query("select id, dbno, outdated from cache"
			 " where version==:a and maxdim==:b limit 1",
			 vsn, maxdim);
  if (!q.next()) {
    qDebug() << "BasicCache: failed to get" << vsn << maxdim;
    throw q;
  }
  quint64 cacheid = q.value(0).toInt();
  int k = q.value(1).toInt();
  bool od = q.value(2).toInt()>0;
  if (outdated_return)
    *outdated_return = od;
  if (k) {
    QByteArray bits(db.simpleQuery(QString("select bits from B%1.blobs"
                                           " where cacheid==:a").arg(k),
                                   cacheid)
		    .toByteArray());
    QBuffer buf(&bits);
    QImageReader reader(&buf, "jpeg");
    return reader.read();
  } else {
    QString fn(constructFilename(vsn, maxdim));
    if (QFile(fn).exists()) 
      return Image16(fn);
    qDebug() << "Missing file " << fn << " from cache";
    return Image16();
  }
}

int BasicCache::bestSize(quint64 vsn, int maxdim) {
  QSqlQuery q = db.query("select maxdim, outdated from cache"
			 " where version==:a", vsn);
  int dbest = 0;
  bool outdated = true;
  bool gotsized = false;
  while (q.next()) {
    int d = q.value(0).toInt();
    bool od = q.value(1).toInt()>0;
    if (od && !outdated) {
      // If I have an up-to-date version, never replace w/ outdated.
    } else if (outdated && !od) {
      // Always replace outdated with up-to-date.
      dbest = d;
      outdated = false;
      gotsized = d>=maxdim;
    } else if (d>=maxdim) {
      // Nice and big, but perhaps too big
      if (d<dbest || !gotsized) {
	dbest = d;
	outdated = od;
	gotsized = true;
      }
    } else {
      // Small, but perhaps better than what we have
      if (d>dbest && !gotsized) {
	dbest = d;
	outdated = od;
      }
    }
  }
  return dbest;  
}

QSize BasicCache::bestSize(quint64 vsn, QSize desired) {
  QSqlQuery q(db.query("select width, height, outdated from cache"
		       " where version==:a", vsn));
  int dbest = 0;
  QSize sbest;
  bool gotsized = false;
  bool outdated = true;
  while (q.next()) {
    int w = q.value(0).toInt();
    int h = q.value(1).toInt();
    int d = w*h;
    bool od = q.value(2).toInt()>0;
    if (od && !outdated) {
      // If I have an up-to-date version, never replace w/ outdated.
    } else if (outdated && !od) {
      // Always replace outdated with up-to-date.
      sbest = QSize(w, h);
      dbest = d;
      outdated = false;
    } else if (w>=desired.width() || h>=desired.height()) {
      // Nice and big, but perhaps too big
      if (d<dbest || !gotsized) {
	sbest = QSize(w, h);
	dbest = d;
	outdated = od;
	gotsized = true;
      }
    } else {
      // Small, but perhaps better than what we have
      if (d>dbest && !gotsized) {
	sbest = QSize(w, h);
	dbest = d;
	outdated = od;
      }
    }
  }
  return sbest;  
}

bool BasicCache::contains(quint64 vsn, bool outdatedOK) {
  QString query = "select count(*) from cache where version==:v";
  if (!outdatedOK)
    query += " and outdated==0";
  query += " limit 1";
  return db.simpleQuery(query, vsn).toInt()>0;
}

QList<QSize> BasicCache::sizes(quint64 vsn, bool outdatedOK) {
  QString query = "select width, height from cache where version==:v";
  if (!outdatedOK)
    query += " and outdated==0";
  query += " order by maxdim";
  QSqlQuery q(db.query(query, vsn));
  
  QList<QSize> lst;
  while (q.next()) {
    int w = q.value(0).toInt();
    int h = q.value(1).toInt();
    lst << QSize(w, h);
  }

  return lst;
}

int BasicCache::maxDim() const {
  return stdsizes[0];
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
  db.query("update cache set outdated=1 where version==:a", vsn);
}
