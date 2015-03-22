// AC_Worker.cpp

#include "AC_Worker.h"
#include "IF_Bank.h"
#include "BasicCache.h"
#include <QVariant>
#include "PDebug.h"
#include "NoResult.h"
#include "AC_ImageHolder.h"

inline uint qHash(PSize const &s) {
  return qHash(QPair<int,int>(s.width(), s.height()));
}

AC_Worker::AC_Worker(PhotoDB const &db, class BasicCache *cache,
		     AC_ImageHolder *holder,
                     QObject *parent):
  QObject(parent), db(db), cache(cache), holder(holder) {
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
  //  pDebug() << "recache " << versions.size();
  try {
    addToDBQueue(versions);
    markReadyToLoad(versions);
    countQueue();
    activateBank();
  } catch (QSqlQuery &q) {
    emit exception("AC_Worker (recache): SqlError: " + q.lastError().text()
		   + " from " + q.lastQuery());
  } catch (NoResult) {
    emit exception("AC_Worker (recache): No result");
  } catch (...) {
    emit exception("AC_Worker (recache): Unknown exception");
  }
}

void AC_Worker::boot() {
  try {
    markReadyToLoad(getSomeFromDBQueue());
    activateBank();
  } catch (QSqlQuery &q) {
    emit exception("AC_Worker (boot): SqlError: " + q.lastError().text()
		   + " from " + q.lastQuery());
  } catch (NoResult) {
    emit exception("AC_Worker (boot): No result");
  } catch (...) {
    emit exception("AC_Worker (boot): Unknown exception");
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
  Transaction t(cache->database());
  for (auto v: versions) {
    cache->markOutdated(v);
    cache->database().query("insert into queue values(:a)", v);
  }
  t.commit();
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
    if (rtlOrder.isEmpty()) {
      pDebug() << "rtlOrder is empty but readyToLoad is not"
	       << readyToLoad.size() << *readyToLoad.begin();
      readyToLoad.clear();
      break;
    }
    quint64 id = rtlOrder.takeFirst();
    if (!readyToLoad.contains(id)) {
      pDebug() << "id" << id << "in rtlOrder but not in readyToLoad";
      continue;
    }
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
  //  pDebug() << "cachePreview" << id;
  if (loaded.contains(id))
    return;
  loaded[id] = img;
  outdatedLoaded << id;
  processLoaded();
}

void AC_Worker::cacheModified(quint64 vsn) {
  Image16 img = holder->getImage(vsn);
  //  pDebug() << "cacheModified" << vsn << img.size() << cache->maxSize();
  if (img.isNull())
    return;
  if (img.size().isLargeEnoughFor(cache->maxSize())) {
    if (beingLoaded.contains(vsn)) 
      invalidatedWhileLoading << vsn;
    else
      N++;
    
    loaded[vsn] = img.scaledToFitIn(cache->maxSize());
    processLoaded();
  } else {
    QSet<quint64> vsns;
    vsns << vsn;
    recache(vsns);
  }
}

void AC_Worker::ensureDBSizeCorrect(quint64 vsn, PSize siz) {
  quint64 photo = db.simpleQuery("select photo from versions where id=:a", vsn)
    .toULongLong();

  QSqlQuery q = db.query("select width, height, orient "
                         "from photos where id=:a", photo);
  if (!q.next())
    throw NoResult(__FILE__, __LINE__);
  int wid = q.value(0).toInt();
  int hei = q.value(1).toInt();
  Exif::Orientation orient = Exif::Orientation(q.value(2).toInt());
  PSize fs = Exif::fixOrientation(siz, orient);

  if (wid!=fs.width() || hei!=fs.height())
    db.query("update photos set width=:a, height=:b where id=:c",
	     fs.width(), fs.height(), photo);
}

void AC_Worker::handleFoundImage(quint64 id, Image16 img, QSize fullSize) {
  // Actually store in cache if we have enough to make it worth while
  // or if readyToLoad is empty and beingLoaded also (after removing
  // this id.)
  // Reactivate the IF_Bank if it is partially idle and we have more.
  //  pDebug() << "AC_Worker::HandleFoundImage" << id << fullSize << img.size();
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

    processLoaded();
  } catch (QSqlQuery &q) {
    emit exception("AC_Worker (handleFoundImage): SqlError: " + q.lastError().text()
		   + " from " + q.lastQuery());
  } catch (NoResult) {
    emit exception("AC_Worker (handleFoundImage): No result");
  } catch (...) {
    emit exception("AC_Worker (handleFoundImage): Unknown exception");
  }
}

void AC_Worker::processLoaded() {
  bool done = loaded.size()>0
    && readyToLoad.isEmpty() && beingLoaded.isEmpty();
  if (done || loaded.size() > threshold) {
    storeLoadedInDB();
    pDebug() << "AC_Worker: Cache progress: " << n << "/" << N << "(" << done <<")";
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
}   

void AC_Worker::sendToBank(quint64 version) {
  QSqlQuery q(db.query("select photo, mods from versions"
                       " where id=:a limit 1", version));
  if (!q.next())
    throw NoResult(__FILE__, __LINE__);
  quint64 photo = q.value(0).toULongLong();
  QString mods = q.value(1).toString();
  q = db.query("select folder, filename, filetype, width, height, orient "
               " from photos where id=:a limit 1", photo);
  if (!q.next())
    throw NoResult(__FILE__, __LINE__);
  quint64 folder = q.value(0).toULongLong();
  QString fn = q.value(1).toString();
  int ftype = q.value(2).toInt();
  int wid = q.value(3).toInt();
  int hei = q.value(4).toInt();
  Exif::Orientation orient = Exif::Orientation(q.value(5).toInt());
  PSize osize = Exif::fixOrientation(PSize(wid,hei), orient);
  QString path = db.folder(folder) + "/" + fn;
  qDebug() << "AC_Worker sendtobank" << version << wid << hei << int(orient) << osize << path;
  int maxdim = cache->maxSize().maxDim();
  bank->findImage(version,
		  path, db.ftype(ftype), orient, osize,
		  mods,
                  maxdim, requests.contains(version));
}

void AC_Worker::storeLoadedInDB() {
  pDebug() << "storeLoadedInDB";
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
  pDebug() << "  storeLoadedinDB done";
  n += loaded.size() - noutdated;
  loaded.clear();
}

void AC_Worker::requestIfEasy(quint64 version, QSize desired) {
  //  pDebug() << "AC_Worker::requestIfEasy" << version << desired;
  try {
    if (loaded.contains(version)) {
      Image16 res = loaded[version].scaledDownToFitIn(desired);
      emit available(version, desired, res);
      return;
    }
    PSize best=cache->bestSize(version, desired);
    if (best.isEmpty())
      return;
    bool od;
    Image16 img = cache->get(version, best, &od).scaledDownToFitIn(desired);
    if (!img.isNull() && !od) // can this happen?
      emit available(version, desired, img);
  } catch (QSqlQuery &q) {
    emit exception("AC_Worker (requestIfEasy): SqlError: " + q.lastError().text()
		   + " from " + q.lastQuery());
  } catch (NoResult) {
    emit exception("AC_Worker (requestIfEasy): No result");
  } catch (...) {
    emit exception("AC_Worker (requestIfEasy): Unknown exception");
  }
}

void AC_Worker::requestImage(quint64 version, QSize desired) {
  if (version==0)
    return;
  //  pDebug() << "AC_Worker::requestImage" << version << desired;
  PSize actual;
  try {
    if (loaded.contains(version)) {
      Image16 res = loaded[version].scaledDownToFitIn(desired);
      emit available(version, desired, res);
      actual = res.size();
    } else if (!(actual=cache->bestSize(version, desired)).isEmpty()) {
      bool od;
      Image16 img = cache->get(version, actual, &od).scaledDownToFitIn(desired);
      if (img.isNull()) // can this happen?
	actual = PSize();
      else
	emit available(version, desired, img);
      if (od)
	actual = PSize();
    }
    if (actual.isLargeEnoughFor(desired)
        || actual.isLargeEnoughFor(cache->maxSize()))
      // We cannot do better than that
      return;

    // We will request it
    if (beingLoaded.contains(version)) {
      // We're getting it already
      requests[version] << desired;
    } else {
      //      pDebug() << "AC_Worker: request: will load " << version << desired;
      if (!mustCache.contains(version))
	N++; /* This is not actually formally correct, because
		it may be that version is in the dbqueue. Worse, by not
		adding this version to the dbqueue, we risk losing count
		later. But adding it to the dbqueue one-by-one is rather
		inefficient.
	     */
      mustCache << version;
      requests[version] << desired;
      readyToLoad << version;
      rtlOrder.push_front(version);
      activateBank();
    }
  } catch (QSqlQuery &q) {
    emit exception("AC_Worker (requestImage): SqlError: " + q.lastError().text()
		   + " from " + q.lastQuery());
  } catch (NoResult) {
    emit exception("AC_Worker (requestImage): No result " + QString::number(version));
  } catch (...) {
    emit exception("AC_Worker (requestImage): Unknown exception");
  }
}

void AC_Worker::respondToRequest(quint64 version, Image16 img) {
  if (img.isNull()) 
    img = Image16(PSize(1, 1));
  PSize s0;
  for (PSize s: requests[version])
    s0 |= s;
  if (img.size().exceeds(s0))
    img = img.scaledToFitIn(s0);
  for (PSize s: requests[version])
    emit available(version, s, img);
  requests.remove(version);
}
