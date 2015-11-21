// AutoCache.cpp

#include "AutoCache.h"
#include "AC_Worker.h"
#include "BasicCache.h"
#include <QMetaType>
#include "PDebug.h"
#include "AC_ImageHolder.h"

AutoCache::AutoCache(PhotoDB *db, QString rootdir, QObject *parent):
  QObject(parent), db(db) {
  setObjectName("AutoCache");
  holder = new AC_ImageHolder(this);
  worker = new AC_Worker(db, rootdir, holder);
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
  connect(worker, SIGNAL(available(quint64, Image16, bool)),
	  this, SIGNAL(available(quint64, Image16, bool)));
  connect(worker, SIGNAL(exception(QString)),
	  this, SIGNAL(exception(QString)));
  connect(this, SIGNAL(forwardCachePreview(quint64, Image16)),
          worker, SLOT(cachePreview(quint64, Image16)));
  connect(this, SIGNAL(forwardCacheModified(quint64)),
          worker, SLOT(cacheModified(quint64)));

  thread.start();
  worker->boot();
}

AutoCache::~AutoCache() {
  thread.quit();
  thread.wait();
}

void AutoCache::cachePreview(quint64 id, Image16 img) {
  emit available(id, img, 1);
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

void AutoCache::cacheModified(quint64 id, Image16 img, quint64 chgid) {
  holder->setImage(id, img);
  emit available(id, img, chgid);
  emit forwardCacheModified(id);
}

void AutoCache::request(quint64 version, QSize desired) {
  emit forwardRequest(version, desired);
}

void AutoCache::requestIfEasy(quint64 version, QSize desired) {
  emit forwardIfEasy(version, desired);
}
