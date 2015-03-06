// AutoCache.h

#ifndef AUTOCACHE_H

#define AUTOCACHE_H

#include <QObject>
#include <QSet>
#include "PhotoDB.h"
#include <QThread>
#include "PSize.h"
#include "Image16.h"

class AutoCache: public QObject {
  Q_OBJECT;
public:
  AutoCache(PhotoDB const &db, QString rootdir, QObject *parent=0);
  virtual ~AutoCache();
  class BasicCache *basicCache() const { return cache; }
public slots:
  void recache(QSet<quint64> versions);
  void recache(quint64 version);
  void request(quint64 version, PSize desired);
  /* The request will be answered through the progressed() signal. If the
     version exists in the cache, the request is answered very quickly, even
     if that means that an outdated or small copy is returned. In that case,
     available() will be emitted again when a better answer is ready.
   */
  void cachePreview(quint64 version, Image16 img);
signals: // public
  void progressed(int n, int N);
  void doneCaching();
  void available(quint64 version, PSize requested, Image16 img);
  /* Emitted in response to a specific request. Note that the image provided
     may not be authoritative: it may be (much) smaller than requested or
     it may even be outdated. As a consequence, AutoCache should not be
     used to obtain images for export.
     The image provided will never exceed the requested size.
   */
  void exception(QString);
signals: // private
  void forwardRecache(QSet<quint64> versions);
  void forwardRequest(quint64 version, PSize desired);
  void forwardCachePreview(quint64 version, Image16 img);
private:
  QThread thread;
  class AC_Worker *worker;
  PhotoDB db;
  class BasicCache *cache;
};

#endif
