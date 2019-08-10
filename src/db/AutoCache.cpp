// AutoCache.cpp

#include "AutoCache.h"
#include "AC_Worker.h"
#include "BasicCache.h"
#include <QMetaType>
#include "PDebug.h"
#include "AC_ImageHolder.h"

AutoCache::AutoCache(SessionDB *db, QString rootdir, QObject *parent):
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
  connect(worker, &AC_Worker::available,
	  [this](quint64 vs, Image16 img, quint64 chgid, QSize fs) {
	    pDebug() << "AC_Worker forward available" << vs << img.size() << chgid << fs;
	    emit available(vs, img, chgid, fs); });
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
  qDebug() << "autocache::cachepreview emit available" << id << img.size();
  emit available(id, img, 1, QSize());
  emit forwardCachePreview(id, img);
}
          
void AutoCache::recache(QSet<quint64> ids) {
  for (auto id: ids)
    holder->dropImage(id);
  
  emit forwardRecache(ids);
}

void AutoCache::recache(quint64 id) {
  QSet<quint64> ids;
  ids << id;
  recache(ids);
}

void AutoCache::cacheModified(quint64 id, Image16 img, quint64 chgid) {
  holder->setImage(id, img);
  pDebug() << "autocache::cachemodified no emit available" 
	   << id << img.size() << chgid;
  //  emit available(id, img, chgid, QSize());
  emit forwardCacheModified(id);
}

void AutoCache::request(quint64 version, QSize desired) {
  emit forwardRequest(version, desired);
}

void AutoCache::requestIfEasy(quint64 version, QSize desired) {
  emit forwardIfEasy(version, desired);
}

void AutoCache::purge(quint64 version) {
  worker->purge(version);
}
