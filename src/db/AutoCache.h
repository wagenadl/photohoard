// AutoCache.h

// Fully documentated for naturaldocs on 11/4/15

#ifndef AUTOCACHE_H

#define AUTOCACHE_H

#include <QObject>
#include <QSet>
#include "PhotoDB.h"
#include <QThread>
#include "PSize.h"
#include "Image16.h"

/* Class: AutoCache
   Top-level class for maintaining an image cache
 */
class AutoCache: public QObject {
  Q_OBJECT;
public:
  /* Constructor: AutoCache
     Arguments:
       db - The database this cache will work for.
       rootdir - The location where the cache files will be stored.
       (For instance, /PATH/FN.cache if /PATH/FN.db is where the database
       is stored.)
   */
  AutoCache(PhotoDB *db, QString rootdir, QObject *parent=0);

  virtual ~AutoCache();

  /* Functions: Slots
     Public slots
  */
public slots:
  /* Function: recache
     
     Causes a set of versions, to be recached. The image will be
     retrieved from the file location. Any images previously provided
     through <cacheModified> will be dropped.

     Most of the actual work is done by <AC_Worker::recache>, which see.

     No specific signal is emitted once the work is complete (other than
     <progressed> and <doneCaching>).

     Argument:
     versions - the versions to be recached
  */
  void recache(QSet<quint64> versions);

  /* Function: recache

     Causes a single version to be recached. Otherwise just like <recache>.
     
     Argument:
     version - the version to be recached
  */
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

  /* Function: request

     Request that an image be provided for a given version at a given size.
     The request will be answered asynchronously through the <available>
     signal.

     If the version exists in the cache, the request is answered very
     quickly, even if that means that an outdated or small copy is
     returned. In that case, <available> will be emitted again when a
     better answer is ready. Otherwise, the image will be cached first.

     Most of the actual work is done in <AC_Worker::requestImage>, which see.

     Arguments:
     version - the ID of the requested version
     desired - the minimum size requested.
   */
  void request(quint64 version, QSize desired);

  /* Function: requestIfEasy
     Like <request>, but only provides an image if it can be done quickly,
     i.e., if it exists in the cache or in memory.

     Most of the actual work is done in <AC_Worker::requestIfEasy>, which see.

     Arguments:
     version - the ID of the requested version
     desired - the minimum size requested.
  */
  void requestIfEasy(quint64 version, QSize desired);

  /* Function: cachePreview

     Stores a preview copy of a version in the cache, that is, a
     thumbnail as stored in some raw files. This functionality is not currently
     actually used. (Even though main connects <Scanner::cacheablePreview>
     to it, <Scanner> never emits that signal.)

     Much of the actual work is done by <AC_Worker::cachePreview>.
     
     Arguments:
     version - the version ID
     img - the thumbnail
  */
  void cachePreview(quint64 version, Image16 img);

  void purge(quint64 version);
  /* PURGE - Throw away cached images for a version */
  
  /* Functions: Signals
     Public signals
   */
signals:
  
  /* Function: cacheProgress
     Emitted periodically to inform user about progress.

     The information provided is not necessarily 100% reliable.

     Arguments:
     n - number of items already processed
     N - total number of items to process
  */
  void progressed(int n, int N);

  /* Function: doneCaching
     Emitted when cache progress reaches n=N.

     After this, cacheProgress will start again with N=0.
  */
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

     Note that the image provided may not be authoritative: it may be
     (much) smaller than requested or it may even be outdated. As a
     consequence, AutoCache should not be used to obtain images for
     export. Note that there may be more than one AVAILABLE response
     to a given request if the initial response could only provide a
     small image. In that case, the second CHGID will be zero.
  */
  
  /* Functions: Private signals
     These signals forward signals connected to our public slots to the
     corresponding slots in the worker thread.
  */
signals:
  // Function: forwardRecache
  void forwardRecache(QSet<quint64> versions);
  // Function: forwardRequest
  void forwardRequest(quint64 version, QSize desired);
  // Function: forwardIfEasy
  void forwardIfEasy(quint64 version, QSize desired);
  // Function: forwardCachePreview
  void forwardCachePreview(quint64 version, Image16 img);
  // Function: forwardCacheModified
  void forwardCacheModified(quint64 version);
private:
  PhotoDB *db; // we do not own
  QThread thread;
  class AC_Worker *worker;
  class AC_ImageHolder *holder;
};

#endif
