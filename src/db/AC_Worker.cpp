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
  connect(bank, SIGNAL(foundImage(quint64, QImage, bool)),
	  this, SLOT(handleFoundImage(quint64, QImage, bool)),
          Qt::QueuedConnection);
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

void AC_Worker::countQueue() {
  N = n + queueLength();
}

int AC_Worker::queueLength() {
  return cache->database().simpleQuery("select count(*) from queue").toInt();
}

void AC_Worker::recache(QSet<quint64> versions) {
  //  qDebug() << "AC_Worker::recache n=" << versions.size();
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
      qDebug() << "rtlOrder is empty but readyToLoad is not" << readyToLoad.size() << *readyToLoad.begin();
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

void AC_Worker::cachePreview(quint64 id, QImage img) {
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

void AC_Worker::ensureDBSizeOK(quint64 vsn, QSize siz) {
  QSqlQuery q(*db);
  q.prepare("select photo, mods from versions where id=:v");
  q.bindValue(":v", vsn);
  if (!q.exec())
    throw q;
  if (!q.next())
    throw NoResult();
  quint64 photo = q.value(0).toULongLong();
  QString mods = q.value(1).toString();
  if (mods!="")
    return;
  
  q.prepare("select width, height "
            " from photos where id=:i");
  q.bindValue(":i", photo);
  if (!q.exec())
    throw q;
  if (!q.next())
    throw NoResult();

  int wid = q.value(0).toInt();
  int hei = q.value(1).toInt();
  if (wid!=siz.width() || hei!=siz.height()) {
    q.prepare("update photos set width=:w, height=:h where id=:i");
    q.bindValue(":i", photo);
    q.bindValue(":w", siz.width());
    q.bindValue(":h", siz.height());
    if (!q.exec())
      throw q;
  }
}

void AC_Worker::handleFoundImage(quint64 id, QImage img, bool isFullSize) {
  // Actually store in cache if we have enough to make it worth while
  // or if readyToLoad is empty and beingLoaded also (after removing
  // this id.)
  // Reactivate the IF_Bank if it is partially idle and we have more.
  //  qDebug() << "HandleFoundImage" << id << img.size();
  try {
    if (requests.contains(id)) {
      if (isFullSize && !img.isNull())
        ensureDBSizeOK(id, img.size());
      respondToRequest(id, img);
    }
    beingLoaded.remove(id);
    outdatedLoaded.remove(id);

    if (invalidatedWhileLoading.contains(id)) {
      invalidatedWhileLoading.remove(id);
    } else if (mustCache.contains(id)) {
      loaded[id] = img;
    }

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
  int maxdim = cache->standardSizes().first();
  if (requests.contains(version)) {
    for (auto s: requests[version]) {
      int md = cache->maxdim(s);
      if (md>maxdim)
	maxdim = md;
    }
  }
  bank->findImage(version, path, mods, ftypes[ftype], orient,
                  maxdim, QSize(wid,hei));
}

void AC_Worker::storeLoadedInDB() {
  Transaction t(cache->database());
  int noutdated = 0;
  for (auto it=loaded.begin(); it!=loaded.end(); it++) {
    quint64 version = it.key();
    QImage img = it.value();
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
    QSize actual(0, 0);
    
    if (loaded.contains(version)) {
      emit available(version, desired, loaded[version]);
      actual = loaded[version].size();
    } else {
      int d = cache->bestSize(version, cache->maxdim(desired));
      if (d>0) {
        bool od;
	QImage img = cache->get(version, d, &od);
	emit available(version, desired, img);
	actual = img.size();
      }
    }

    if (actual.width()>=desired.width() || actual.height()>=desired.height())
      return; // easy, we're done
    
    // We will probably have to request it to get the right size.

    if (beingLoaded.contains(version)) {
      // If it is already being loaded, we are liable to get
      // a copy that is too small.
      bool contained = false;
      for (auto s: requests[version]) {
        if (s.width()>=desired.width() && s.height()>=desired.height()) {
          contained = true;
          break;
        }
      }
      requests[version] << desired;
      if (contained) {
        // That's easy, we'll get a large enough version
      } else {
        invalidatedWhileLoading << version;
        if (!readyToLoad.contains(version)) {
          readyToLoad << version;
          rtlOrder.push_front(version);
        }
      }
    } else {
      if (BasicCache::maxdim(actual) < cache->maxDim()) {
        mustCache << version;
      }
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

void AC_Worker::respondToRequest(quint64 version, QImage img) {
  if (img.isNull()) 
    img = QImage(QSize(1, 1), QImage::Format_RGB32);
  for (QSize s: requests[version])
    emit available(version, s, img);
  requests.remove(version);
}