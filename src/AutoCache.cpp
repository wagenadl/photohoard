// AutoCache.cpp

#include "AutoCache.h"
#include "AC_Worker.h"

AutoCache::AutoCache(PhotoDB const &db, QString rootdir, QObject *parent):
  QObject(parent), db(db) {
  cache = new BasicCache(this);
  worker = new AC_Worker(this);
  worker->moveToThread(&thread);
  connect(&thread, SIGNAL(finished()), worker, SLOT(deleteLater()));
  connect(this, SIGNAL(forwardRecache(QSet<quint64>)),
          worker, SLOT(recache(QSet<quint64>)));
  connect(this, SIGNAL(forwardRecache(quint64)),
          worker, SLOT(recache(quint64)));
}

AutoCache::~AutoCache() {
  thread.quit();
  thread.wait();
}
          
void AutoCache::recache(QSet<quint64> ids) {
  emit forwardRecache(ids);
}

void AutoCache::recache(quint64 id) {
  QSet<quint64> ids;
  ids << id;
  emit forwardRecache(ids);
}

    

