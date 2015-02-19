// AutoCache.h

#ifndef AUTOCACHE_H

#define AUTOCACHE_H

#include <QObject>
#include <QSet>
#include "PhotoDB.h"
#include <QThread>
#include <QSize>
#include <QImage>

class AutoCache: public QObject {
  Q_OBJECT;
public:
  AutoCache(PhotoDB const &db, QString rootdir, QObject *parent=0);
  virtual ~AutoCache();
public slots:
  void recache(QSet<quint64> ids);
  void recache(quint64 id);
  void request(quint64 version, QSize desired);
  void cachePreview(quint64 id, QImage img);
signals: // public
  void progressed(int n, int N);
  void doneCaching();
  void available(quint64 version, QSize requested, QImage img);
  void exception(QString);
signals: // private
  void forwardRecache(QSet<quint64> ids);
  void forwardRequest(quint64 version, QSize desired);
  void forwardCachePreview(quint64 id, QImage img);
private:
  QThread thread;
  class AC_Worker *worker;
  PhotoDB db;
  class BasicCache *cache;
};

#endif
