// AC_Worker.cpp

#include "AC_Worker.h"
#include "IF_Bank.h"
#include "BasicCache.h"
#include <QVariant>
#include <QDebug>
#include "NoResult.h"

inline uint qHash(QSize const &s) {
  return qHash(QPair<int,int>(s.width(), s.height()));
}

AC_Worker::AC_Worker(PhotoDB const &db, class BasicCache *cache,
                     QObject *parent):
  QObject(parent), db(db), cache(cache) {
  setObjectName("AC_Worker");
  readFTypes();
  threshold = 100;
  bank = new IF_Bank(4, this); // number of threads comes from where?
  bank->setMaxDim(0); // cache->maxDim());
  connect(bank, SIGNAL(foundImage(quint64, QImage)),
	  this, SLOT(handleFoundImage(quint64, QImage)));
  connect(bank, SIGNAL(exception(QString)),
	  this, SIGNAL(exception(QString)));
  n = 0;
  N = 0;
}

void AC_Worker::readFTypes() {
  QSqlQuery q(*db);
  q.prepare("select id, stdext from filetypes");
  if (!q.exec()) {
    qDebug() << "Could not select extensions";
    throw q;
  }
  while (q.next()) 
    ftypes[q.value(0).toInt()] = q.value(1).toString();
}  

void AC_Worker::recache(QSet<quint64> versions) {
  try {
    qDebug() << " AC_Worker::recache";
    addToDBQueue(versions);
    markReadyToLoad(versions);
    activateBank();
  } catch (QSqlQuery &q) {
    emit exception("AC_Worker: SqlError: " + q.lastError().text()
		   + " from " + q.lastQuery());
  } catch (...) {
    emit exception("AC_Worker: Unknown exception");
  }
}

void AC_Worker::boot() {
  try {
    markReadyToLoad(getSomeFromDBQueue());
    activateBank();
  } catch (QSqlQuery &q) {
    emit exception("AC_Worker: SqlError: " + q.lastError().text()
		   + " from " + q.lastQuery());
  } catch (...) {
    emit exception("AC_Worker: Unknown exception");
  }
}

QSet<quint64> AC_Worker::getSomeFromDBQueue(int maxres) {
  QSqlQuery qq(*cache->database());
  qq.prepare("select version from queue limit :m");
  qq.bindValue(":m", QVariant(maxres));
  if (!qq.exec())
    throw qq;
  QSet<quint64> ids;
  while (qq.next()) 
    ids << qq.value(0).toULongLong();
  return ids;
}
 
void AC_Worker::markReadyToLoad(QSet<quint64> versions) {
  for (auto v: versions) {
    if (beingLoaded.contains(v)) 
      invalidatedWhileLoading << v;
    else if (loaded.contains(v))
      loaded.remove(v);
    if (!readyToLoad.contains(v)) {
      readyToLoad << v;
      mustCache << v;
      rtlOrder << v;
      N++;
    }
  }
}  

void AC_Worker::addToDBQueue(QSet<quint64> versions) {
  if (versions.isEmpty())
    return;
  Transaction t(cache->database());
  QSqlQuery q(*cache->database());
  for (auto v: versions) {
    q.prepare("update cache set outdated=1 where version==:i");
    q.bindValue(":i", v);
    if (!q.exec())
      throw q;
    q.prepare("insert into queue values(:i)");
    q.bindValue(":i", v);
    if (!q.exec())
      throw q;
  }
  t.commit();
  qDebug() << "recache committed";
}

void AC_Worker::activateBank() {
  QList<quint64> notyetready;
  int K = bank->availableThreads();
  while (K>0 && !readyToLoad.isEmpty()) {
    quint64 id = rtlOrder.takeFirst();
    if (!readyToLoad.contains(id))
      continue;
    if (invalidatedWhileLoading.contains(id)) {
      notyetready << id;
    } else {
      readyToLoad.remove(id);
      beingLoaded.insert(id);
      sendToBank(id);
      K--;
    }
  }
  for (auto id: notyetready) 
    rtlOrder.push_front(id); // put back what we could not do now
}
    
void AC_Worker::handleFoundImage(quint64 id, QImage img) {
  // Actually store in cache if we have enough to make it worth while
  // or if readyToLoad is empty and beingLoaded also (after removing
  // this id.)
  // Reactivate the IF_Bank if it is partially idle and we have more.
  try {
    if (requests.contains(id))
      respondToRequest(id, img);

    beingLoaded.remove(id);

    if (invalidatedWhileLoading.contains(id)) {
      invalidatedWhileLoading.remove(id);
    } else if (mustCache.contains(id)) {
      loaded[id] = img;
    }

    bool done = readyToLoad.isEmpty() && beingLoaded.isEmpty();
    if (done || loaded.size() > threshold)
      storeLoadedInDB();
    emit cacheProgress(n, N);

    if (done) {
      markReadyToLoad(getSomeFromDBQueue());
      if (readyToLoad.isEmpty()) {
	emit doneCaching();
	n = 0;
	N = 0;
      }
    } 
  
    activateBank();
  } catch (QSqlQuery &q) {
    emit exception("AC_Worker: SqlError: " + q.lastError().text()
		   + " from " + q.lastQuery());
  } catch (...) {
    emit exception("AC_Worker: Unknown exception");
  }
}

void AC_Worker::sendToBank(quint64 version) {
  QSqlQuery q(*db);
  q.prepare("select photo, ver from versions where id=:v");
  q.bindValue(":v", version);
  if (!q.exec())
    throw q;
  if (!q.next())
    throw NoResult();
  quint64 photo = q.value(0).toULongLong();
  int ver = q.value(1).toInt();
  q.prepare("select folder, filename, filetype, orient "
	    " from photos where id=:i");
  q.bindValue(":i", photo);
  if (!q.exec())
    throw q;
  if (!q.next())
    throw NoResult();
  quint64 folder = q.value(0).toULongLong();
  QString fn = q.value(1).toString();
  int ftype = q.value(2).toInt();
  //    int wid = q.value(3).toInt();
  //    int hei = q.value(4).toInt();
  Exif::Orientation orient = Exif::Orientation(q.value(5).toInt());
  if (!folders.contains(folder)) {
    q.prepare("select pathname from folders where id=:i");
    q.bindValue(":i", folder);
    if (!q.exec())
      throw q;
    if (!q.next())
      throw NoResult();
    folders[folder] = q.value(0).toString();
  }
  QString path = folders[folder] + "/" + fn;

  bank->findImage(version, path, ver, ftypes[ftype], orient);
}

void AC_Worker::storeLoadedInDB() {
  Transaction t(cache->database());
  QSqlQuery q(*cache->database());
  for (auto it=loaded.begin(); it!=loaded.end(); it++) {
    quint64 version = it.key();
    QImage img = it.value();
    cache->add(version, img);

    q.prepare("delete from queue where version==:v");
    q.bindValue(":v", version);
    if (!q.exec())
      throw q;
  }
  t.commit();
  n += loaded.size();
  loaded.clear();
}

void AC_Worker::requestImage(quint64 version, QSize desired) {
  try {
    QSize actual(0, 0);
    if (loaded.contains(version)) {
      emit available(version, desired, loaded[version]);
      actual = loaded[version].size();
    } else {
      int d = cache->bestSize(version, cache->maxdim(desired));
      if (d>0) {
	QImage img = cache->get(version, d);
	emit available(version, desired, img);
	actual = img.size();
      }
    }
    qDebug() << "AC_Worker::requestImage"
	     << version << desired << actual << beingLoaded.contains(version);
    if (actual.width()<desired.width() && actual.height()<desired.height()) {
      requests[version] << desired;
      if (!beingLoaded.contains(version)) {
	// If it is already being loaded, we may be liable to get
	// a copy that is too small. But I am not sure.
	readyToLoad << version;
	rtlOrder.push_front(version);
	activateBank();
      }
    }
  } catch (QSqlQuery &q) {
    emit exception("AC_Worker: SqlError: " + q.lastError().text()
		   + " from " + q.lastQuery());
  } catch (...) {
    emit exception("AC_Worker: Unknown exception");
  }
}

void AC_Worker::respondToRequest(quint64 version, QImage img) {
  qDebug() << "AC_Worker::respondToRequest " << version << img.size();
  for (QSize s: requests[version])
    emit available(version, s, img);
  requests.remove(version);
}