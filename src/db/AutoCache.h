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
  AutoCache(PhotoDB *db, QString rootdir, QObject *parent=0);
  virtual ~AutoCache();
  //  class BasicCache *basicCache() const { return cache; }
public slots:
  void recache(QSet<quint64> versions);
  void recache(quint64 version);
  void cacheModified(quint64 version, Image16 img, quint64 chgid=1);
  /* CACHEMODIFIED - Store a modified version of an image
     If you have changed a version's settings, it suffices to ask the
     AUTOCACHE to RECACHE it at its leisure, but if you have already
     painstakingly calculated the new image, you can also offer it
     directly to AUTOCACHE through CACHEMODIFIED(version, img). This
     will cause the IMG to be stored in the cache as the new image for
     VERSION. (If IMG is not large enough for AUTOCACHE's taste, it
     might still calculate a larger version later.)
     This causes an AVAILABLE signal to be emitted. By calling
     CACHEMODIFIED(version, img, chgid), you can directly specify the
     CHGID for the AVAILABLE signal. (It defaults to 1.)
   */
  void request(quint64 version, QSize desired);
  /* The request will be answered through the progressed() signal. If the
     version exists in the cache, the request is answered very quickly, even
     if that means that an outdated or small copy is returned. In that case,
     available() will be emitted again when a better answer is ready.
   */
  void requestIfEasy(quint64 version, QSize desired);
  void cachePreview(quint64 version, Image16 img);
signals: // public
  void progressed(int n, int N);
  void doneCaching();
  void available(quint64 version, Image16 image, quint64 chgid);
  /* AVAILABLE - Emitted whenever a version is newly available
     AVAILABLE(version, image, chgid) is emitted to report that the given
     VERSION has just been added to the cache, or has been successfully
     retrieved from the cache.
     The IMAGE may be full size or smaller,
     depending on whether it was retrieved from the cache, newly generated,
     or just offered through CACHEMODIFIED. Normally, only one AVAILABLE
     signal is emitted per cache event for the image, even if several REQUESTs
     had been made (e.g., for different sizes).
     The CHGID is positive if this signals a new or modified version
     of the image, or zero if it is just a cache retrieval.
   */
  void exception(QString);
signals: // private
  void forwardRecache(QSet<quint64> versions);
  void forwardRequest(quint64 version, QSize desired);
  void forwardIfEasy(quint64 version, QSize desired);
  void forwardCachePreview(quint64 version, Image16 img);
  void forwardCacheModified(quint64 version);
private:
  PhotoDB *db; // we do not own
  QThread thread;
  class AC_Worker *worker;
  class AC_ImageHolder *holder;
};

#endif
