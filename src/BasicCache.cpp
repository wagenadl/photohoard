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
  readConfig();
  qDebug() << "BasicCache constructed 1";
  db.query("pragma synchronous = 0");
}

BasicCache::BasicCache(QDir root, Database const &db, QObject *parent):
  QObject(parent), root(root), db(db) {
  readConfig();
  qDebug() << "BasicCache constructed 2";
  this->db.query("pragma synchronous = 0");
}

BasicCache::~BasicCache() {
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

QImage BasicCache::sufficientSize(QImage const &img) {
  int d = maxdim(img.size());
  if (d>stdsizes[0])
    return img.scaled(QSize(stdsizes[0], stdsizes[0]), Qt::KeepAspectRatio);
  else
    return img;
}

void BasicCache::add(quint64 vsn, QImage img, bool instantlyOutdated) {
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
  QSqlQuery q(db.query("select maxdim from cache"
		       " where version==:a and outdated>0 and infile>0", vsn));
  while (q.next()) {
    int d = q.value(0).toInt();
    QFile(constructFilename(vsn, d)).remove();
  }
  db.query("delete from cache where version==:a and outdated>0", vsn);
}

void BasicCache::addToCache(quint64 vsn, QImage const &img,
                            bool instantlyOutdated) {
  QBuffer buf;
  QImageWriter writer(&buf, "jpeg");
  writer.write(img);

  bool infile = buf.data().size() >= memthresh;
  int d = maxdim(img.size());

  QSqlQuery q = db.query("select id, infile from cache"
			 " where version==:a and maxdim==:b", vsn, d);
  if (q.next()) {
    // preexist
    quint64 cacheid = q.value(0).toInt();
    bool wasinfile = q.value(1).toInt()>0;
    if (infile) {
      if (!wasinfile) {
	db.query("delete from blobs where cacheid==:a", cacheid);
	db.query("update cache set infile=1"
		 " where version==:a and maxdim==:b", vsn, d);
      }
      QFile f(constructFilename(vsn, d));
      if (f.open(QFile::WriteOnly)) 
	f.write(buf.data());
      else
	throw std::system_error(std::error_code());
    } else {
      if (wasinfile) {
	db.query("insert into blobs (cacheid, bits) values (:a, :b)",
		 cacheid, buf.data()).lastInsertId().toInt();
	db.query("update cache set infile=0"
		 " where version==:b and maxdim==:c", vsn, d);
	QFile(constructFilename(vsn, d)).remove();
      } else {
	db.query("update blobs set bits=:a where cacheid==:b",
		 buf.data(), cacheid);
      }
    }
  } else {
    // new result
    if (infile) {
      QFile f(constructFilename(vsn, d));
      if (f.open(QFile::WriteOnly)) 
	f.write(buf.data());
      else
	throw std::system_error(std::error_code());
      db.query("insert into cache (version, maxdim, outdated, infile)"
	       " values (:a, :b, :c, 1)", vsn, d, instantlyOutdated?1:0);
    } else {
      quint64 cacheid = db.query("insert into cache"
				 " (version, maxdim, outdated, infile)"
				 " values (:a, :b, :c, 0)",
				 vsn, d, instantlyOutdated?1:0)
	.lastInsertId().toInt();
      db.query("insert into blobs (cacheid, bits) values (:a, :b)",
	       cacheid, buf.data());
    }
  }
}

void BasicCache::remove(quint64 vsn) {
  QSqlQuery q(db.query("select maxdim, infile from cache"
		       " where version==:a", vsn));
  while (q.next()) {
    int d = q.value(0).toInt();
    bool infile = q.value(1).toInt()>0;
    if (infile)
      QFile(constructFilename(vsn, d)).remove();
  }
  db.query("delete from cache where version==:a", vsn);
}

QImage BasicCache::get(quint64 vsn, int maxdim, bool *outdated_return) {
  QSqlQuery q = db.query("select id, infile, outdated from cache"
			 " where version==:a and maxdim==:b limit 1",
			 vsn, maxdim);
  if (!q.next()) {
    qDebug() << "BasicCache: failed to get" << vsn << maxdim;
    throw q;
  }
  quint64 cacheid = q.value(0).toInt();
  bool infile = q.value(1).toInt()>0;
  bool od = q.value(2).toInt()>0;
  if (outdated_return)
    *outdated_return = od;
  if (infile) {
    QString fn(constructFilename(vsn, maxdim));
    if (QFile(fn).exists()) 
      return QImage(fn);
    qDebug() << "Missing file " << fn << " from cache";
    return QImage();
  } else {
    QByteArray bits(db.simpleQuery("select bits from blobs"
				   " where cacheid==:a", cacheid)
		    .toByteArray());
    QBuffer buf(&bits);
    QImageReader reader(&buf, "jpeg");
    return reader.read();
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
