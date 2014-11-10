// AC_Worker.cpp

#include "AC_Worker.h"
#include "IF_Bank.h"

AC_Worker::AC_Worker(PhotoDB const &db, class BasicCache *cache,
                     QObject *parent):
  QObject(parent), db(db), cache(cache) {
  bank = new IF_Bank(4, this); // number of threads comes from where?
  bank->setMaxDim(cache->maxDim());
}

void AC_Worker::recache(QSet<quint64> ids) {
}

void AC_Worker::handleFoundImage(quint64 id, QImage img) {
}
