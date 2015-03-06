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
  threshold = 100;
  bank = new IF_Bank(4, this); // number of threads comes from where?
  connect(bank, SIGNAL(foundImage(quint64, Image16, QSize)),
	  this, SLOT(handleFoundImage(quint64, Image16, QSize)),
          Qt::QueuedConnection);
  connect(bank, SIGNAL(exception(QString)),
	  this, SIGNAL(exception(QString)));
  n = 0;
  N = 0;
}

void AC_Worker::countQueue() {
  N = n + queueLength();
}

int AC_Worker::queueLength() {
  return cache->database().simpleQuery("select count(*) from queue").toInt();
}

void AC_Worker::recache(QSet<quint64> versions) {
  try {
    addToDBQueue(versions);
    markReadyToLoad(versions);
    countQueue();
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
    }
  }
}  

void AC_Worker::addToDBQueue(QSet<quint64> versions) {
  if (versions.isEmpty())
    return;
  //  qDebug() << "addToDBQueue" << versions.size();
  Transaction t(cache->database());
  //  qDebug() << "  transaction started";
  for (auto v: versions) {
    //    qDebug() << "  working on " << v;
    cache->markOutdated(v);
    cache->database().query("insert into queue values(:a)", v);
  }
  t.commit();
  //  qDebug() << "  transaction committed";
}

void AC_Worker::activateBank() {
  QSet<quint64> notyetready; // This will collect ones that are already
  // being loaded but need to be loaded again. We cannot start them over
  // until they are done, because we don't have a way to track which
  // version of a request is which.
  
  int K = bank->availableThreads();
  if (requests.isEmpty()) {
    int K1 = bank->totalThreads()/2;
    if (K1<1)
      K1 = 1;
    if (K>K1)
      K = K1;
  }
  QSet<quint64> tobesent;
  while (K>0 && !readyToLoad.isEmpty()) {
    if (rtlOrder.isEmpty()) 
      qDebug() << "rtlOrder is empty but readyToLoad is not"
	       << readyToLoad.size() << *readyToLoad.begin();
    quint64 id = rtlOrder.takeFirst();
    if (!readyToLoad.contains(id))
      continue;
    if (invalidatedWhileLoading.contains(id)) {
      notyetready << id;
    } else {
      readyToLoad.remove(id);
      beingLoaded.insert(id);
      tobesent << id;
      K--;
    }
  }
  for (auto id: notyetready) 
    rtlOrder.push_front(id); // put back what we cannot do now

  for (auto id: tobesent)
    sendToBank(id);
}

void AC_Worker::cachePreview(quint64 id, Image16 img) {
  if (loaded.contains(id))
    return;
  loaded[id] = img;
  outdatedLoaded << id;
  bool done = readyToLoad.isEmpty() && beingLoaded.isEmpty();
  if (done || loaded.size() > threshold) {
    storeLoadedInDB();
    qDebug() << "AC_Worker: Cache preview progress: " << n << "/" << N;
    emit cacheProgress(n, N);
  }
}

void AC_Worker::ensureDBSizeCorrect(quint64 vsn, QSize siz) {
  quint64 photo = db.simpleQuery("select photo from versions where id=:a", vsn)
    .toULongLong();

  QSqlQuery q = db.query("select width, height from photos where id=:a", photo);
  if (!q.next())
    throw NoResult();
  int wid = q.value(0).toInt();
  int hei = q.value(1).toInt();

  if (wid!=siz.width() || hei!=siz.height())
    db.query("update photos set width=:a, height=:b where id=:c",
	     siz.width(), siz.height(), photo);
}

void AC_Worker::handleFoundImage(quint64 id, Image16 img, QSize fullSize) {
  // Actually store in cache if we have enough to make it worth while
  // or if readyToLoad is empty and beingLoaded also (after removing
  // this id.)
  // Reactivate the IF_Bank if it is partially idle and we have more.
  //  qDebug() << "HandleFoundImage" << id << img.size();
  try {
    if (!fullSize.isEmpty())
      ensureDBSizeCorrect(id, fullSize); // Why should this be needed?
    if (requests.contains(id)) 
      respondToRequest(id, img);
    beingLoaded.remove(id);
    outdatedLoaded.remove(id);

    if (invalidatedWhileLoading.contains(id)) 
      invalidatedWhileLoading.remove(id);
    else if (mustCache.contains(id))
      loaded[id] = img;
    
    bool done = loaded.size()>0
      && readyToLoad.isEmpty() && beingLoaded.isEmpty();
    if (done || loaded.size() > threshold) {
      storeLoadedInDB();
      qDebug() << "AC_Worker: Cache progress: " << n << "/" << N;
      emit cacheProgress(n, N);
    }
    
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
  QSqlQuery q(db.query("select photo, mods from versions"
                       " where id=:a limit 1", version));
  if (!q.next())
    throw NoResult();
  quint64 photo = q.value(0).toULongLong();
  QString mods = q.value(1).toString();
  q = db.query("select folder, filename, filetype, width, height, orient "
               " from photos where id=:a limit 1", photo);
  if (!q.next())
    throw NoResult();
  quint64 folder = q.value(0).toULongLong();
  QString fn = q.value(1).toString();
  int ftype = q.value(2).toInt();
  int wid = q.value(3).toInt();
  int hei = q.value(4).toInt();
  Exif::Orientation orient = Exif::Orientation(q.value(5).toInt());
  QString path = db.folder(folder) + "/" + fn;
  int maxdim = cache->standardSizes().first();
  bank->findImage(version,
		  path, db.ftype(ftype), orient, QSize(wid, hei),
		  mods,
                  maxdim, requests.contains(version));
}

void AC_Worker::storeLoadedInDB() {
  Transaction t(cache->database());
  int noutdated = 0;
  for (auto it=loaded.begin(); it!=loaded.end(); it++) {
    quint64 version = it.key();
    Image16 img = it.value();
    bool outd =  outdatedLoaded.contains(version);
    cache->add(version, img, outd);
    if (outd)
      noutdated++;

    cache->database().query("delete from queue where version==:a", version);
  }
  t.commit();
  n += loaded.size() - noutdated;
  loaded.clear();
}

void AC_Worker::requestImage(quint64 version, QSize desired) {
  //  qDebug() << "AC_Worker::requestImage" << version << desired;
  try {
    if (loaded.contains(version)) {
      emit available(version, desired, loaded[version]);
      return;
    } 

    int d = cache->bestSize(version, cache->maxdim(desired));
    if (d>0) {
      bool od;
      Image16 img = cache->get(version, d, &od);
      emit available(version, desired, img);
      if (!od)
	return;
    }

    // We will have to request it
    if (beingLoaded.contains(version)) {
      // We'll get it
      requests[version] << desired;
    } else {
      mustCache << version;
      requests[version] << desired;
      readyToLoad << version;
      rtlOrder.push_front(version);
      activateBank();
    }
  } catch (QSqlQuery &q) {
    emit exception("AC_Worker: SqlError: " + q.lastError().text()
		   + " from " + q.lastQuery());
  } catch (...) {
    emit exception("AC_Worker: Unknown exception");
  }
}

void AC_Worker::respondToRequest(quint64 version, Image16 img) {
  if (img.isNull()) 
    img = Image16(QSize(1, 1));
  for (QSize s: requests[version])
    emit available(version, s, img);
  requests.remove(version);
}
