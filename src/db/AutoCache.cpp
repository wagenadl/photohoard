// AutoCache.cpp

#include "AutoCache.h"
#include "AC_Worker.h"
#include "BasicCache.h"
#include <QMetaType>
#include <QDebug>

AutoCache::AutoCache(PhotoDB const &db, QString rootdir, QObject *parent):
  QObject(parent), db(db) {
  setObjectName("AutoCache");
  cache = new BasicCache(rootdir, this);
  worker = new AC_Worker(db, cache);
  worker->moveToThread(&thread);
  
  qRegisterMetaType< QSet<quint64> >("QSet<quint64>");
  
  connect(&thread, SIGNAL(finished()), worker, SLOT(deleteLater()));
  connect(this, SIGNAL(forwardRecache(QSet<quint64>)),
          worker, SLOT(recache(QSet<quint64>)));
  connect(this, SIGNAL(forwardRequest(quint64, QSize)),
	  worker, SLOT(requestImage(quint64, QSize)));
  connect(this, SIGNAL(forwardIfEasy(quint64, QSize)),
	  worker, SLOT(requestIfEasy(quint64, QSize)));
  connect(worker, SIGNAL(cacheProgress(int,int)),
	  this, SIGNAL(progressed(int,int)));
  connect(worker, SIGNAL(doneCaching()),
	  this, SIGNAL(doneCaching()));
  connect(worker, SIGNAL(available(quint64, QSize, Image16)),
	  this, SIGNAL(available(quint64, QSize, Image16)));
  connect(worker, SIGNAL(exception(QString)),
	  this, SIGNAL(exception(QString)));
  connect(this, SIGNAL(forwardCachePreview(quint64, Image16)),
          worker, SLOT(cachePreview(quint64, Image16)));
  connect(this, SIGNAL(forwardCacheModified(quint64, Image16)),
          worker, SLOT(cacheModified(quint64, Image16)));

  thread.start();
  worker->boot();
}

AutoCache::~AutoCache() {
  thread.quit();
  thread.wait();
}

void AutoCache::cachePreview(quint64 id, Image16 img) {
  emit forwardCachePreview(id, img);
}
          
void AutoCache::recache(QSet<quint64> ids) {
  emit forwardRecache(ids);
}

void AutoCache::recache(quint64 id) {
  QSet<quint64> ids;
  ids << id;
  emit forwardRecache(ids);
}

void AutoCache::cacheModified(quint64 id, Image16 img) {
  emit forwardCacheModified(id, img);
}

void AutoCache::request(quint64 version, QSize desired) {
  emit forwardRequest(version, desired);
}

void AutoCache::requestIfEasy(quint64 version, QSize desired) {
  emit forwardIfEasy(version, desired);
}
