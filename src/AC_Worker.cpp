// AC_Worker.cpp

#include "AC_Worker.h"
#include "IF_Bank.h"

AC_Worker::AC_Worker(PhotoDB const &db, class BasicCache *cache,
                     QObject *parent):
  QObject(parent), db(db), cache(cache) {
  bank = new IF_Bank(4, this); // number of threads comes from where?
  bank->setMaxDim(cache->maxDim());
  n = 0;
  N = 0;
}

void AC_Worker::recache(QSet<quint64> versions) {
  addToDBQueue(versions);
  markReadyToLoad(versions);
  activateBank();
}

void AC_Worker::boot() {
  markReadyToLoad(getSomeFromDBQueue(100));
  activateBank();
}

QSet<quint64> AC_Worker::getSomeFromDBQueue(int maxres) {
  QSqlQuery qq(*cache->database());
  qq.prepare("select version from queue limit :m");
  qq.bindValue(":m", maxres);
  if (!qq.exec())
    throw qq;
  QSet<quint64> ids;
  while (qq.next()) 
    ids << qq.value(0).toULongLong();
  return ids;
}
 
void AC_Worker::markReadyToLoad(QSet<quint64> versions) {
  foreach (auto v: versions) {
    if (beingLoaded.contains(v)) 
      invalidatedWhileLoading << v;
    else if (loaded.contains(v))
      loaded.remove(v);
    if (!rtlSet.contains(v)) {
      readyToLoad << v;
      rtlSet << v;
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
    q.bindValue(":i", version);
    if (!q.exec())
      throw q;
    q.prepare("insert into queue values(:i)");
    q.bindValue(":i", version);
    if (!q.exec())
      throw q;
  }
  t.commit();
  qDebug() << "recache committed";
}

void AC_Worker::activateBank() {
  QSet<quint64> notyetready;
  int K = bank->availableThreads();
  while (K>0 && !readyToLoad.isEmpty()) {
    quint64 id = readyToLoad.dequeue();
    if (invalidatedWhileLoading.contains(id)) {
      notyetready << id;
    } else {
      rtlSet.remove(id);
      beingLoaded.insert(id);
      sendToBank(id);
      K--;
    }
  }
  for (auto id: notyetready) 
    readyToLoad.push_front(id); // put back what we could not do now
}
    
void AC_Worker::handleFoundImage(quint64 id, QImage img) {
  // Actually store in cache if we have enough to make it worth while
  // or if readyToLoad is empty and beingLoaded also (after removing
  // this id.)
  // Reactivate the IF_Bank if it is partially idle and we have more.

  //if (urgentlyNeeded(id)) {
    // ...
    // emit ...
  //}

  beingLoaded.remove(id);

  if (invalidatedWhileLoading.contains(id)) {
    invalidatedWhileLoading.remove(id);
  } else {
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
}
