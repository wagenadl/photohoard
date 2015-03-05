// AutoCache.h

#ifndef AUTOCACHE_H

#define AUTOCACHE_H

#include <QObject>
#include <QSet>
#include "PhotoDB.h"
#include <QThread>
#include <QSize>
#include "Image16.h"

class AutoCache: public QObject {
  Q_OBJECT;
public:
  AutoCache(PhotoDB const &db, QString rootdir, QObject *parent=0);
  virtual ~AutoCache();
  class BasicCache *basicCache() const { return cache; }
public slots:
  void recache(QSet<quint64> ids);
  void recache(quint64 id);
  void request(quint64 version, QSize desired);
  //  void requestOriginal(quint64 version);
  //  void requestScaledOriginal(quint64 version, QSize desired);
  void cachePreview(quint64 id, Image16 img);
signals: // public
  void progressed(int n, int N);
  void doneCaching();
  void available(quint64 version, QSize requested, Image16 img);
  void originalAvailable(quint64 version, Image16 img);
  void scaledOriginalAvailable(quint64 version, QSize requested, Image16 img);
  void exception(QString);
signals: // private
  void forwardRecache(QSet<quint64> ids);
  void forwardRequest(quint64 version, QSize desired);
  void forwardCachePreview(quint64 id, Image16 img);
  void forwardRequestOriginal(quint64 version);
  void forwardRequestScaledOriginal(quint64 version, QSize desired);
private:
  QThread thread;
  class AC_Worker *worker;
  //  QThread ofthread;
  //  class OriginalFinder *ofinder;
  PhotoDB db;
  class BasicCache *cache;
};

#endif
