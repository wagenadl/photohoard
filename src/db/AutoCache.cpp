// AutoCache.cpp

#include "AutoCache.h"
#include "AC_Worker.h"
#include "BasicCache.h"
#include <QMetaType>
#include <QDebug>
// #include "OriginalFinder.h"

AutoCache::AutoCache(PhotoDB const &db, QString rootdir, QObject *parent):
  QObject(parent), db(db) {
  setObjectName("AutoCache");
  cache = new BasicCache(rootdir, this);
  worker = new AC_Worker(db, cache);
  worker->moveToThread(&thread);
  //  ofinder = new OriginalFinder(db, cache);
  //  ofinder->moveToThread(&ofthread);
  
  qRegisterMetaType< QSet<quint64> >("QSet<quint64>");
  
  connect(&thread, SIGNAL(finished()), worker, SLOT(deleteLater()));
  connect(this, SIGNAL(forwardRecache(QSet<quint64>)),
          worker, SLOT(recache(QSet<quint64>)));
  connect(this, SIGNAL(forwardRequest(quint64, QSize)),
	  worker, SLOT(requestImage(quint64, QSize)));
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

//  connect(&ofthread, SIGNAL(finished()), ofinder, SLOT(deleteLater()));
//  connect(this, SIGNAL(forwardRequestOriginal(quint64)),
//	  ofinder, SLOT(requestOriginal(quint64)));
//  connect(this, SIGNAL(forwardRequestScaledOriginal(quint64, QSize)),
//	  ofinder, SLOT(requestScaledOriginal(quint64, QSize)));
//  connect(ofinder, SIGNAL(originalAvailable(quint64, Image16)),
//	  SIGNAL(originalAvailable(quint64, Image16)));
//  connect(ofinder, SIGNAL(scaledOriginalAvailable(quint64, QSize, Image16)),
//	  SIGNAL(scaledOriginalAvailable(quint64, QSize, Image16)));
//  connect(ofinder, SIGNAL(exception(QString)),
//	  this, SIGNAL(exception(QString)));
  
  thread.start();
  worker->boot();
  //  ofthread.start();
}

AutoCache::~AutoCache() {
  thread.quit();
  //  ofthread.quit();
  thread.wait();
  //  ofthread.wait();
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

void AutoCache::request(quint64 version, QSize desired) {
  emit forwardRequest(version, desired);
}


//void AutoCache::requestOriginal(quint64 version) {
//  emit forwardRequestOriginal(version);
//}
//
//
//void AutoCache::requestScaledOriginal(quint64 version, QSize desired) {
//  emit forwardRequestScaledOriginal(version, desired);
//}
//
