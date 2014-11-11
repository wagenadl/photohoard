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
  N += versions.size();
  // Now add to readyToLoad and activate the IF_Bank if it is partially idle.
}

void AC_Worker::addToDBQueue(QSet<quint64> versions) {
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

void AC_Worker::handleFoundImage(quint64 id, QImage img) {
  // Actually store in cache if we have enough to make it worth while
  // or if readyToLoad is empty and beingLoaded also (after removing
  // this id.)
  // Reactivate the IF_Bank if it is partially idle and we have more.
}
