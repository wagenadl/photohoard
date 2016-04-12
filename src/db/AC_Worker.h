// AC_Worker.h

// Fully documentated for naturaldocs on 11/4/15

#ifndef AC_WORKER_H

#define AC_WORKER_H

#include <QObject>

#include <QSet>
#include "Image16.h"
#include <QMap>

/* Class: AC_Worker
   Worker thread for <AutoCache>.

   In the current implementation, there is precisely one AC_Worker for
   a given AutoCache, of which there is one per <PhotoDB>. However,
   AC_Worker uses an entire bank of <ImageFinders> (contained in an
   <IF_Bank>) to do the time consuming work of reading image files from
   disk.   
*/
class AC_Worker: public QObject {
  Q_OBJECT;
public:
  /* Constructor: AC_Worker

     Arguments:
     db - Photo database this cache works for
     rootdir - as for <AutoCache::AutoCache>
     holder - object to be used to funnel images from
     <AutoCache::cacheModified> to our <cacheModified>.
   */
  AC_Worker(class SessionDB const *db, QString rootdir,
	    class AC_ImageHolder *holder,
            QObject *parent=0);
  virtual ~AC_Worker();

public slots:
  /* Function: boot
     Get initial list of cachable items from db and activate bank of
     <ImageFinders>.
  */
  void boot(); 

  /* Function: recache
     Backend for <AutoCache::recache>.
     Causes the given versions to be dropped from the cache and reloaded
     from disk.

     The versions don't get reloaded instantly, rather, they are added to
     the queue through <addToDBQueue>. They are also marked ready to load
     through <markReadyToLoad>.

     The <IF_Bank> is activated through <activateBank> if it wasn't
     already active.

     Arguments:
     versions - IDs of versions to be reloaded
  */
  void recache(QSet<quint64> versions);

  /* Function: requestImage
     Backend for <AutoCache::request>.
     Requests that an image (identified by its version ID) by retrieved
     from the cache, or, if it is not there, added to the cache.

     If the image exists in the cache (at any scale) or is available
     in memory (because it is currently being added to the cache), an
     <available> signal is immediately emitted. If the image does not
     exist in the cache, or if a size is requested that exceeds what
     is available, the <IF_Bank> is activated and the <available>
     signal is emitted asynchronously whenever the image is ready.

     Arguments:
     version - ID of version requested
     desired - minimum size wanted
  */
  void requestImage(quint64 version, QSize desired);

  /* Function: requestIfEasy
     Backend for <AutoCache::requestIfEasy>.
     Like <requestImage>, except that the <IF_Bank> is never involved. That
     is, either an <available> signal is emitted right away, or not at all.

     No indication is provided if the image cannot be provided.
   */
  void requestIfEasy(quint64 version, QSize desired);

  /* Function: cachePreview
     Backend for <AutoCache::cachePreview>. Stores a thumbnail of a version
     in the cache.

     This functionality is not currently actually used, and it may be buggy.
     In particular, if a better image already exists in the cache, it is
     overwritten. However, if a better image has just been loaded and is
     about to be copied into the cache, the thumbnail is ignored.

     Sets <onlyPreviewLoaded> on the given version, so that a better copy should
     eventually be loaded.
       
     Arguments:
     version - the version ID
     img - the thumbnail
  */     
  void cachePreview(quint64 vsn, Image16 img);

  /* Function: cacheModified
     Backend for <AutoCache::cacheModified>.
     Transfers an image from the <AC_ImageHolder> to the cache.
     If there is no image associated with the given version in the holder,
     nothing happens.

     Arguments:
     vsn - the version ID
  */
  void cacheModified(quint64 vsn);

  void purge(quint64 vsn);
  /* PURGE - Throw away cached images for a version */

private slots:
  /* Function: handleFoundImage (slot)
     Responds when an image has been found by one of the <ImageFinders>.

     Normally connected to <IF_Bank::foundImage>.

     If the image was requested through <requestImage> as opposed to simply
     suggested through <recache>, an <available> signal is emited (through
     <respondToRequest>), even if the image was invalidated during loading,
     a signal is still emitted.
     
     Unless invalidated while loading, the image will be stored in the
     cache, not necessarily immediately, but when a sufficient batch
     of images has been loaded into memory (through <processLoaded>, which
     also removes the version from the queue).

     Arguments:
     version - ID of the image just loaded
     img - actual image (possibly larger or smaller than requested)
     fullSize - maximum size that this image could be produced at
  */
  void handleFoundImage(quint64 version, Image16 img, QSize fullSize);
  
signals:
  /* Function: cacheProgress (signal)
     Emitted periodically to inform user about progress.

     The information provided is not necessarily 100% reliable.

     Arguments:
     n - number of items already processed
     N - total number of items to process
  */
  void cacheProgress(int n, int N);

  /* Function: doneCaching (signal)
     Emitted when cache progress reaches n=N.

     After this, cacheProgress will start again with N=0.
  */
  void doneCaching();

  void available(quint64 version, Image16 img, quint64 chgid);
  /* AVAILABLE - Emitted when a requested image is available.
     See AUTOCACHE::AVAILABLE for details.
  */
private:
  void makeAvailable(quint64 version, Image16 img);
  /* MAKEAVAILABLE - Emit AVAILABLE signals for the given image.
  */

  /* Function: getSomeFromDBQueue

     Simply gets a limited number of version IDs from the head of the
     queue. These IDs can then be marked ready to load through
     <markReadyToLoad>.

     Argument:
     maxres - maximum number of IDs to be returned. (Fewer will be returned
     if fewer are available.)
  */
  QSet<quint64> getSomeFromDBQueue(int maxres=100);

  /* Function: markReadyToLoad
     
     Adds a set of versions to <readyToLoad>. Any versions that were
     already being loaded are marked <invalidatedWhileLoading>.

     Argument:
     versions - set of version IDs
  */
  void markReadyToLoad(QSet<quint64> versions);

  /* Function: addToDBQueue

     Simply adds the given versions to the queue for later processing.
     Note that the queue is stable across quitting/restarting the program,
     since it is stored in the (cache) DB.

     Argument:
     versions - set of version IDs
  */
  void addToDBQueue(QSet<quint64> versions);

  /* Function: activateBank
     
     Moves a few versions from <readyToLoad> to <beingLoaded> and
     hands them over to the <IF_Bank> through <sendToBank>.

     No more versions are moved than to fill the bank's available threads.

     Versions that are <invalidatedWhileLoading> are never moved,
     since those cannot be requested again until the previous request
     completes. (The present implementation of <IF_Bank> has no way to
     cancel an outstanding request.)
   */
  void activateBank();

  /* Function: sendToBank
     
     Simply hand control of a version over to the <IF_Bank>. Does not update
     internal variables such as <beingLoaded>.
  */
  void sendToBank(quint64 version);

  /* Function: storeLoadedInDB

    Unconditionally stores all loaded images in the cache.

    Cached images are marked "outdated" based on <onlyPreviewLoaded>.
   */
  void storeLoadedInDB();

  /* Function: ensureDBSizeCorrect

     Ensures that sizes in the versions table are correct

     Arguments:
     vsn - version ID
     s - size of image (orientation-corrected, rather than as in file)
  */
  void ensureDBSizeCorrect(quint64 vsn, PSize s);

  /* Function: countQueue

     Ensures that the variable <N> is correct given the current <n> and the
     actual length of the queue as stored in the database.
   */
  void countQueue();

  /* Function: queueLength

     Returns the number of items in the queue of to-be-cached versions.
  */
  int queueLength() const;

  /* Function: processLoaded

     Stores loaded images in cache through <storeLoadedInDB> if work
     is complete or if we have collected enough. Emits <cacheProgress>
     and <doneCaching> signals. Calls <activateBank>.

     Does not emit <available> signals.
   */
  void processLoaded();
private:
  class SessionDB *db; // we own
  class BasicCache *cache;
  class IF_Bank *bank;

  // Variable: n
  // Number of versions cached since last <doneCaching>
  int n;

  // Variable: N
  // Number of versions scheduled to be cached since last <doneCaching>
  int N;

  // Variable: threshold
  // Max. number of images to load before committing to cache
  int threshold;

  // Variable: bytethreshold
  // Max. combined byte size of images to load before committing to cache
  int bytethreshold;

  // Variable: readyToLoad
  // The set of versions that could be sent off to the <IF_Bank> for loading.
  QSet<quint64> readyToLoad;

  // Variable: rtlOrder
  // Priority of versions in the <readyToLoad> set
  //
  // (Highest priority is first in list.)
  QList<quint64> rtlOrder;

  // Variable: mustCache
  // Set of images in <readyToLoad>, <beingLoaded>, or <loaded> that
  // should be cached at the end of the process.
  QSet<quint64> mustCache;

  // Variable: beingLoaded
  // Set of all the IDs currently being loaded (i.e., under control of the
  // IF_Bank).
  QSet<quint64> beingLoaded;

  // Variable: invalidatedWhileLoading
  // Subset of <beingLoaded> that is invalidated by a new call to recache()
  QSet<quint64> invalidatedWhileLoading;
  QMap<quint64, Image16> loaded;
  int loadedmemsize;

  QSet<quint64> hushup;
  // HUSHUP - Set of IDs that should not generate an AVAILABLE signals

  // Variable: onlyPreviewLoaded
  // Subset of <loaded> for which we merely loaded a preview thumbnail
  QSet<quint64> onlyPreviewLoaded;

  // Variable: requests
  // Map of version IDs to set of desired sizes for all explicitly requested
  // items (as per <requestImage>).
  QMap<quint64, PSize> requests;
  class AC_ImageHolder *holder;
};

#endif
