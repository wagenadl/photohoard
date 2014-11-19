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
}

BasicCache::BasicCache(QDir root, Database const &db, QObject *parent):
  QObject(parent), root(root), db(db) {
  readConfig();
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
    return new BasicCache(root, db);
  } catch (...) {
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

void BasicCache::add(quint64 id, QImage img) {
  int d = maxdim(img.size());
  if (d<=stdsizes[0]) 
    // cache image directly: it is smaller than our largest desired size
    addToCache(id, img);
  for (auto s: stdsizes) {
    if (s<d) {
      img = img.scaled(QSize(s,s), Qt::KeepAspectRatio);
      addToCache(id, img);
    }
  }
}

void BasicCache::addToCache(quint64 id, QImage const &img) {
  QBuffer buf;
  QImageWriter writer(&buf, "jpeg");
  writer.write(img);
  QSqlQuery q(*db);
  bool infile = buf.data().size() >= memthresh;
  int d = maxdim(img.size());

  q.prepare("select id from cache where version==:v and maxdim==:d");
  q.bindValue(":v", id);
  q.bindValue(":d", d);
  if (!q.exec())
    throw q;
  QString rowid;
  if (q.next()) {
    // preexist
    rowid = q.value(0).toString();
    q.prepare("update cache "
	      " set width=:w, height=:h, outdated=:o, infile=:i, bits=:b "
	      " where version==:v and maxdim==:d");
  } else {
    q.prepare("insert into cache "
	      " (version, width, height, maxdim, outdated, infile, bits) "
	      " values (:v, :w, :h, :d, :o, :i, :b)");
  }
  q.bindValue(":v", id);
  q.bindValue(":w", img.width());
  q.bindValue(":h", img.height());
  q.bindValue(":d", d);
  q.bindValue(":o", 0);
  q.bindValue(":i", infile);
  q.bindValue(":b", infile ? QByteArray() : buf.data());
  if (!q.exec())
    throw q;
  if (rowid.isNull())
    rowid = q.lastInsertId().toString();
  if (infile) {
    QFile f(root.absoluteFilePath(rowid + ".jpg"));
    if (f.open(QFile::WriteOnly)) 
      f.write(buf.data());
    else
      throw std::system_error(std::error_code());
  } else {
    if (root.exists(rowid + ".jpg"))
      root.remove(rowid+".jpg");
  }
}
  

void BasicCache::remove(quint64 id) {
  QSqlQuery q(*db);
  q.prepare("select id, infile from cache where version==:v");
  q.bindValue(":v", id);
  if (!q.exec()) {
    qDebug() << "BasicCache::remove: Could not execute select";
    return;
  }
  while (q.next()) {
    QString rowid = q.value(0).toString();
    bool infile = q.value(1).toInt();
    if (infile)
      root.remove(rowid+".jpg");
  }
  q.prepare("delete from cache where version==:v");
  q.bindValue(":v", id);
  if (!q.exec()) {
    qDebug() << "BasicCache::remove: Could not execute delete";
    return;
  }
  
}

 QImage BasicCache::get(quint64 id, int maxdim, bool *outdated_return) {
  QSqlQuery q(*db);
  q.prepare("select id, infile, bits, outdated from cache "
	    " where version==:v and maxdim==:d");
  q.bindValue(":v", id);
  q.bindValue(":d", maxdim);
  if (!q.exec()) {
    qDebug() << "BasicCache::get: Could not execute select";
    return QImage();
  }
  if (!q.next()) {
    qDebug() << "BasicCache::get: Could not get row";
    return QImage();
  }
  QString rowid = q.value(0).toString();
  bool infile = q.value(1).toBool();
  if (outdated_return)
    *outdated_return = q.value(3).toBool();
  
  if (infile) {
    return QImage(root.absoluteFilePath(rowid + ".jpg"));
  } else {
    QByteArray bits(q.value(2).toByteArray());
    QBuffer buf(&bits);
    QImageReader reader(&buf, "jpeg");
    return reader.read();
  }  
}

int BasicCache::bestSize(quint64 id, int maxdim) {
  QSqlQuery q(*db);
  q.prepare("select maxdim, outdated from cache where version==:v");
  q.bindValue(":v", id);
  if (!q.exec()) {
    qDebug() << "BasicCache::bestSize: Could not execute select";
    return 0;
  }
  int dbest = 0;
  bool outdated = true;
  bool gotsized = false;
  while (q.next()) {
    int d = q.value(0).toInt();
    bool od = q.value(1).toBool();
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

QSize BasicCache::bestSize(quint64 id, QSize desired) {
  QSqlQuery q(*db);
  q.prepare("select width, height, outdated from cache where version==:v");
  q.bindValue(":v", id);
  if (!q.exec()) {
    qDebug() << "BasicCache::bestSize: Could not execute select";
    throw q;
  }
  int dbest = 0;
  QSize sbest;
  bool gotsized = false;
  bool outdated = true;
  while (q.next()) {
    int w = q.value(0).toInt();
    int h = q.value(1).toInt();
    int d = w*h;
    bool od = q.value(2).toBool();
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

bool BasicCache::contains(quint64 id, bool outdatedOK) {
  QSqlQuery q(*db);
  QString query = "select count(*) from cache where version==:v";
  if (!outdatedOK)
    query += " and outdated==0";
  q.prepare(query);
  q.bindValue(":v", id);
  if (!q.exec() || !q.next()) {
    qDebug() << "BasicCache::contains: Could not execute query.";
    return false;
  }
  return q.value(0).toInt()>0;
}

QList<QSize> BasicCache::sizes(quint64 id, bool outdatedOK) {
  QSqlQuery q(*db);
  QString query = "select width, height from cache where version==:v";
  if (!outdatedOK)
    query += " and outdated==0";
  query += " order by maxdim";
  q.prepare(query);
  q.bindValue(":v", id);
  
  QList<QSize> lst;
  
  if (!q.exec()) {
    qDebug() << "BasicCache::contains: Could not execute query.";
    return lst;
  }

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
    
