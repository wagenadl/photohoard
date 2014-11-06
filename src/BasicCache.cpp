// BasicCache.cpp

#include "BasicCache.h"
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QBuffer>
#include <QImageWriter>
#include <QImageReader>

BasicCache::BasicCache(QString rootdir): root(rootdir) {
  root.makeAbsolute();
  db = QSqlDatabase::addDatabase("QSQLITE", root.absolutePath());
  db.setDatabaseName(root.absoluteFilePath("cache.db"));
  if (!db.open()) 
    qDebug() << "BasicCache: Could not open database";
  readConfig();
}

BasicCache::BasicCache(QDir root, QSqlDatabase const &db):
  root(root), db(db) {
  readConfig();
}

BasicCache::~BasicCache() {
  QSqlDatabase::removeDatabase(root.absolutePath());
}

void BasicCache::readConfig() {
  QSqlQuery q(db);
  q.exec("select bytes from memthresh");
  if (q.next()) {
    memthresh = q.value(0).toInt();
  } else {
    memthresh = 200000;
    qDebug() << "Could not read memory threshold from db";
  }

  q.exec("select maxdim from sizes");
  while (q.next())
    stdsizes << q.value(0).toInt();
  qSort(stdsizes.begin(), stdsizes.end(), qGreater<int>());
}
  
bool BasicCache::ok() const {
  return db.isOpen();
}

BasicCache *BasicCache::create(QString rootdir) {
  QDir root(rootdir);
  root.makeAbsolute();
  
  if (root.exists()) {
    qDebug() << "BasicCache::create: Directory preexists.";
    return NULL;
  }

  if (!root.mkdir(root.absolutePath())) {
    qDebug() << "BasicCache::create: Could not create directory.";
    return NULL;
  }

  try {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE",
						root.absolutePath());
    db.setDatabaseName(root.absoluteFilePath("cache.db"));
    if (!db.open()) {
      qDebug() << "BasicCache::create: Could not create and open database.";
      throw 0;
    }
    
    QFile sqlf(":/setupcache.sql");
    if (!sqlf.open(QFile::ReadOnly)) {
      qDebug() << "BasicCache::create: Could not open my SQL code.";
      throw 0;
    }
    QString sql = QString(sqlf.readAll());
    QStringList cmds = sql.split(";");
    QSqlQuery q(db);
    for (auto c: cmds) {
      c.replace(QRegExp("--.*\n"), "");
      c.replace(QRegExp("^\\s+"), "");
      c.replace(QRegExp("\\s+$"), "");
      if (c.isEmpty())
	continue;
      if (!q.exec(c)) {
	qDebug() << "BasicCache::create: Failed to execute SQL code.";
	qDebug() << "  " << c;
	qDebug() << "  " << q.lastError().text();
	throw 0;
      }
    }
    return new BasicCache(root, db);
  } catch (int) {
    root.remove("cache.db");
    root.rmdir(root.absolutePath());
    return NULL;
  }
}

int BasicCache::maxdim(QSize const &s) {
  int w = s.width();
  int h = s.height();
  return w>h ? w : h;
}

void BasicCache::add(quint64 id, QImage img) {
  int d = maxdim(img.size());
  if (d<stdsizes[0]) 
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
  QSqlQuery q(db);
  bool infile = buf.data().size() >= memthresh;
  int d = maxdim(img.size());

  q.prepare("select id from cache where version==:v and maxdim==:d");
  q.bindValue(":v", id);
  q.bindValue(":d", d);
  if (!q.exec()) {
    qDebug() << "Could not test for preexistence";
    return;
  }
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
  if (!q.exec()) {
    qDebug() << "Could not insert " << id << " into cache at "
	     << img.width() << "x" << img.height();
    qDebug() << "  " << q.lastError().text();
    return;
  }
  if (rowid.isNull())
    rowid = q.lastInsertId().toString();
  qDebug() << "Inserted " << id << " into cache at "
	   << img.width() << "x" << img.height() << " as " << rowid; 
  if (infile) {
    QFile f(root.absoluteFilePath(rowid + ".jpg"));
    if (f.open(QFile::WriteOnly)) 
      f.write(buf.data());
    else
      qDebug() << "Failed to write to file";
  } else {
    if (root.exists(rowid + ".jpg"))
      root.remove(rowid+".jpg");
  }
}
  

void BasicCache::remove(quint64 id) {
  QSqlQuery q(db);
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
  QSqlQuery q(db);
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
  QSqlQuery q(db);
  q.prepare("select maxdim, outdated from cache where version==:v");
  q.bindValue(":v", id);
  if (!q.exec()) {
    qDebug() << "BasicCache::bestSize: Could not execute select";
    return 0;
  }
  int dbest = 0;
  bool outdated = true;
  while (q.next()) {
    int d = q.value(0).toInt();
    bool od = q.value(1).toBool();
    if (od && !outdated) {
      // If I have an up-to-date version, never replace w/ outdated.
    } else if (outdated && !od) {
      // Always replace outdated with up-to-date.
      dbest = d;
      outdated = false;
    } else if (d>=maxdim) {
      // Nice and big, but perhaps too big
      if (d<dbest || dbest<maxdim) {
	dbest = d;
	outdated = od;
      }
    } else {
      // Small, but perhaps better than what we have
      if (d>dbest && dbest<maxdim) {
	dbest = d;
	outdated = od;
      }
    }
  }
  return dbest;  
}

bool BasicCache::contains(quint64 id, bool outdatedOK) {
  QSqlQuery q(db);
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
  QSqlQuery q(db);
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

    
  
 
 
