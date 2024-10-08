// AC_Worker.cpp

#include "AC_Worker.h"

#include "IF_Bank.h"
#include "BasicCache.h"
#include <QVariant>
#include "PDebug.h"
#include "AC_ImageHolder.h"
#include "AllAdjustments.h"
#include "Here.h"
#include "SessionDB.h"
#include "ImgAvg.h"

inline uint qHash(PSize const &s) {
  return qHash(QPair<int,int>(s.width(), s.height()));
}

AC_Worker::AC_Worker(SessionDB const *db0, QString rootdir,
		     AC_ImageHolder *holder,
                     QObject *parent):
  QObject(parent), holder(holder) {
  pDebug() << "AC_Worker" << this;
  setObjectName("AC_Worker");
  db = new SessionDB;
  db->clone(*db0);
  cache = new BasicCache;
  cache->open(rootdir);
  threshold = 100;
  bytethreshold = 200*1000*1000;
  loadedmemsize = 0;
  bank = new IF_Bank(4, this); // number of threads comes from where?
  connect(bank, SIGNAL(foundImage(quint64, Image16, QSize)),
	  this, SLOT(handleFoundImage(quint64, Image16, QSize)),
          Qt::QueuedConnection);
  n = 0;
  countQueue();
}

AC_Worker::~AC_Worker() {
  db->close();
  delete db;
  cache->close();
  delete cache;
}

void AC_Worker::countQueue() {
  N = n + queueLength();
}

int AC_Worker::queueLength() const {
  DBReadLock lock(cache->database());
  return cache->database()->simpleQuery("select count(*) from queue").toInt();
}

void AC_Worker::recache(QSet<quint64> versions) {
  addToDBQueue(versions);
  markReadyToLoad(versions);
  countQueue();
  activateBank();
}

void AC_Worker::boot() {
  markReadyToLoad(getSomeFromDBQueue());
  activateBank();
}

QSet<quint64> AC_Worker::getSomeFromDBQueue(int maxres) {
  DBReadLock lock(cache->database());
  QSqlQuery qq = cache->database()
    ->constQuery("select version from queue limit :a", maxres);
  QSet<quint64> ids;
  while (qq.next()) 
    ids << qq.value(0).toULongLong();
  return ids;
}
 
void AC_Worker::markReadyToLoad(QSet<quint64> versions) {
  for (auto v: versions) {
    if (beingLoaded.contains(v)) {
      invalidatedWhileLoading << v;
    } else if (loaded.contains(v)) {
      loadedmemsize -= loaded[v].byteCount();
      loaded.remove(v);
    }
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
  pDebug() << "acworker addtodbq" << versions.size();
  for (auto v: versions) {
    cache->markOutdated(v);
    cache->database()->query("insert into queue values(:a)", v);
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
      COMPLAIN("rtlOrder is empty but readyToLoad is not");
      readyToLoad.clear();
      break;
    }
    quint64 id = rtlOrder.takeFirst();
    if (!readyToLoad.contains(id)) {
      COMPLAIN("id" + QString::number(id)
               + "in rtlOrder but not in readyToLoad");
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
  if (loaded.contains(id))
    return;
  loaded[id] = img;
  loadedmemsize += img.byteCount();
  onlyPreviewLoaded << id;
  processLoaded();
}

void AC_Worker::cacheModified(quint64 vsn) {
  Image16 img = holder->getImage(vsn);
  if (img.isNull())
    return;
  if (img.size().isLargeEnoughFor(cache->maxSize())) {
    if (beingLoaded.contains(vsn)) 
      invalidatedWhileLoading << vsn;
    else
      N++;
    requests.remove(vsn);
    
    loaded[vsn] = img.scaledToFitSnuglyIn(cache->maxSize());
    processLoaded();
  } else {
    hushup << vsn;
    QSet<quint64> vsns;
    vsns << vsn;
    recache(vsns);
  }
}

void AC_Worker::ensureDBSizeCorrect(quint64 vsn, PSize siz) {
  // siz must be the orientation-corrected size for the given version
  int wid, hei;
  Exif::Orientation orient;
  qulonglong photo;
  PSize fs;
  { DBReadLock lock(db);
    QSqlQuery q = db->constQuery("select width, height, orient, photos.id"
                            " from versions"
                            " inner join photos on versions.photo==photos.id"
                            " where versions.id==:a", vsn);
    ASSERT(q.next());
    wid = q.value(0).toInt();
    hei = q.value(1).toInt();
    orient = Exif::Orientation(q.value(2).toInt());
    photo = q.value(3).toULongLong();
    fs = Exif::fixOrientation(siz, orient);
  }

  if (wid!=fs.width() || hei!=fs.height()) {
    DBWriteLock lock(db);
    pDebug() << "AC_Worker::ensureDBSize";
    db->query("update photos set width=:a, height=:b where id=:c",
	     fs.width(), fs.height(), photo);
  }
}

void AC_Worker::handleFoundImage(quint64 id, Image16 img, QSize fullSize) {
  // Actually store in cache if we have enough to make it worth while
  // or if readyToLoad is empty and beingLoaded also (after removing
  // this id.)
  // Reactivate the IF_Bank if it is partially idle and we have more.

  if (!fullSize.isEmpty())
    ensureDBSizeCorrect(id, fullSize);
  /* Above is needed, because images without EXIF data may not have their
   * size information stored in the DB yet. */
  
  if (!hushup.contains(id)
      && (!invalidatedWhileLoading.contains(id) || requests.contains(id))) 
    makeAvailable(id, img, fullSize);

  beingLoaded.remove(id);
  onlyPreviewLoaded.remove(id);

  if (invalidatedWhileLoading.contains(id)) {
    invalidatedWhileLoading.remove(id);
  } else if (mustCache.contains(id)) {
    loaded[id] = img;
    loadedmemsize += img.byteCount();
  }

  processLoaded();
}

void AC_Worker::processLoaded() {
  bool done = loaded.size()>0
    && readyToLoad.isEmpty() && beingLoaded.isEmpty();
  if (done || loaded.size() > threshold || loadedmemsize>bytethreshold) {
    storeLoadedInDB();
    pDebug() << "AC_Worker: Cache progress: " << n << "/" << N
             << "(" << done <<")";
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

void AC_Worker::sendToBank(quint64 vsn) {
  AllAdjustments adjs;
  adjs.readFromDB(vsn, *db);

  quint64 folder;
  QString fn;
  int ftype;
  int wid, hei;
  Exif::Orientation orient;
  { DBReadLock lock(db);
    QSqlQuery q
      = db->constQuery("select folder, filename, filetype, width, height, orient "
                  " from versions"
                  " inner join photos on versions.photo==photos.id"
                  " where versions.id==:a limit 1", vsn);
    ASSERT(q.next());
    folder = q.value(0).toULongLong();
    fn = q.value(1).toString();
    ftype = q.value(2).toInt();
    wid = q.value(3).toInt();
    hei = q.value(4).toInt();
    orient = Exif::Orientation(q.value(5).toInt());
  }
  
  PSize osize = Exif::fixOrientation(PSize(wid,hei), orient);
  QString path = db->folder(folder) + "/" + fn;
  int maxdim = cache->maxSize().maxDim();
  bank->findImage(vsn,
		  path, db->ftype(ftype), orient, osize,
		  adjs,
                  maxdim, requests.contains(vsn));
}

void AC_Worker::storeLoadedInDB() {
  Transaction t(cache->database());
  pDebug() << "accworker storelid" << loaded.size();
  int noutdated = 0;
  for (auto it=loaded.begin(); it!=loaded.end(); it++) {
    quint64 version = it.key();
    Image16 img = it.value();
    bool outdated = onlyPreviewLoaded.contains(version);
    cache->markOutdated(version);
    cache->add(version, img, outdated);
    if (outdated)
      noutdated++;
    cache->database()->query("delete from queue where version==:a", version);
  }
  t.commit();
  n += loaded.size() - noutdated;
  loaded.clear();
  loadedmemsize = 0;
}

void AC_Worker::requestIfEasy(quint64 version, QSize desired) {
  if (loaded.contains(version)) {
    Image16 res = loaded[version].scaledDownToFitIn(desired);
    emit available(version, res, cache->isOutdated(version) ? 1 : 0, QSize());
    return;
  }
  PSize best=cache->bestSize(version, desired);
  if (best.isEmpty())
    return;
  bool od;
  Image16 img = cache->get(version, best, &od).scaledDownToFitIn(desired);
  if (!img.isNull() && !od)
    emit available(version, img, 0, QSize());
}

void AC_Worker::requestImage(quint64 version, QSize desired) {
  if (version==0)
    return;

  PSize actual;
  if (loaded.contains(version)) {
    Image16 res = loaded[version].scaledDownToFitIn(desired);
    emit available(version, res, false, QSize());
    actual = res.size();
  } else {
    actual=cache->bestSize(version, desired);
    if (!actual.isEmpty()) {
      bool outdated;
      Image16 img = cache->get(version, actual, &outdated)
        .scaledDownToFitIn(desired);
      if (img.isNull()) {
        // can this happen?
        actual = PSize();
      } else {
        emit available(version, img, false, QSize());
      }
      if (outdated)
        actual = PSize();
    }
  }
  if (actual.isLargeEnoughFor(desired)
      || actual.isLargeEnoughFor(cache->maxSize()))
    // We cannot do better than that
    return;
  
  // We will request it
  if (beingLoaded.contains(version)) {
    // We're getting it already
    requests[version] |= desired;
  } else {
    // We must get it
    if (!mustCache.contains(version))
      N++; /* This is not actually formally correct, because it may
              be that version is already in the dbqueue. Worse, by
              not adding this version to the dbqueue, we risk losing
              count later. But adding it to the dbqueue one-by-one
              is rather inefficient.
           */
    mustCache << version;
    requests[version] |= desired;
    if (readyToLoad.contains(version))
      rtlOrder.removeAll(version); // we'll push to front
    else
      readyToLoad.insert(version);
    rtlOrder.push_front(version);
    activateBank();
  }
}

void AC_Worker::makeAvailable(quint64 version, Image16 img, QSize fullsize) {
  if (img.isNull()) 
    img = Image16(PSize(1, 1));
  PSize s = requests[version];
  img = img.scaledDownToFitIn(s);
  emit available(version, img.scaledDownToFitIn(s),
		 cache->isOutdated(version), fullsize);
  requests.remove(version);
}

void AC_Worker::purge(quint64 version) {
  cache->remove(version);
}
