// Adjuster.h

#ifndef ADJUSTER_H

#define ADJUSTER_H

#include "Image16.h"
#include "Adjustments.h"
#include <QRect>
#include "PSize.h"
#include <QObject>
#include "AdjusterTile.h"

/* Class: Adjuster

   Adjuster is a simple factory that can apply adjustments to an image.
   It is meant to be used in a synchronous fashion and does not use signals
   to return results.
*/
class Adjuster: public QObject {
  /* ADJUSTER - Workhorse for applying adjustments to an image
     ADJUSTER is a simple, nonthreaded factory for calculating adjusted
     versions of a photo.
     At present, ADJUSTER doesn't know about layers. This will have to be
     changed, because layers are important, not just for INTERRUPTABLEADJUSTER,
     but for other clients of ADJUSTER as well.
   */
  Q_OBJECT;
public:
  // Constructor: Adjuster
  Adjuster(QObject *parent=0);
  virtual ~Adjuster();
  void setMaxThreads(int);
  // SETMAXTHREADS - Specfiy how many threads this adjuster may use
  void clear();
  // CLEAR - Drop entire stack
  bool isEmpty() const;
  // ISEMPTY - Return true if no image is set
  void setOriginal(Image16 const &image);
  /* SETORIGINAL - Loads a new original image into the adjuster
     SETORIGINAL(image) loads the given IMAGE into the adjuster, dropping
     any stages that previously existed. */
  void setReduced(Image16 const &image, PSize originalSize);
  /* SETREDUCED - Loads a new original image at less than full size
     SETREDUCED(image, originalSize) loads a new original image into the
     adjuster, just like SETORIGINAL, but not at its full
     resolution.
     The image may be at any resolution (smaller than the
     original); the original size must be specified through
     ORIGINALSIZE. SETREDUCED determines the scale
     of the reduced image from the actual and original sizes, taking the
     average of the X and Y scale factors.
  */
  Image16 retrieveFull(Adjustments const &settings);
  /* RETRIEVEFULL - Retrieves a version of the image with settings applied.
     RETRIEVEFULL(settings) retrieves an image, at full resolution, to which
     all the given SETTINGS have been applied.
     RETRIEVEFULL fails (returning a null image) if we do not have the
     full resolution.
     Results are cached so that asking for the same image twice is fast.
  */
  Image16 retrieveReduced(Adjustments const &settings, PSize maxSize);
  /* RETRIEVEREDUCED - As RETRIEVEFULL, but does not need full res.
     RETRIEVEREDUCED(settings, maxSize) retrieves a version of the
     image with SETTINGS applied and (usually)
     reduced in resolution to fit within the given MAXSIZE.
     This always succeeds, even if we don't have enough resolution to give
     the requested size. (The best available size is returned in that case.)
     Note that a larger than requested image may be returned if that is
     quicker. RETRIEVEREDUCED will never upscale an image.
  */

  PSize maxAvailableSize(Adjustments const &settings) const;
  /* MAXAVAILABLESIZE - Maximum size of an image that we can offer
     MAXAVAILABLESIZE(settings) returns the maximum size of an image
     that we have on offer for the given SETTINGS. The image itself must
     be obtained through RETRIEVEREDUCED. Note that the max available size
     depends both on the size of the original (as per SETREDUCED) and on
     the SETTINGS. (Especially, of course, crop and rotation settings.)
  */     
  PSize neededScaledOriginalSize(Adjustments const &settings,
                                 PSize desired) const;
  /* NEEDEDSCALEDORIGINALSIZE - Minimum size required to produce desired version
     NEEDEDSCALEDORIGINALSIZE(settings, desired) calculates how large
     a scaled version of an original image is needed to produce an
     adjusted version (as specified in SETTINGS) at the DESIRED final
     size. This applies to the currently loaded image.
   */
  Image16 retrieveROI(Adjustments const &settings, QRect roi);
  /* RETRIEVEROI - Retrieves a tile from the image with settings applied.
     RETRIEVEROI(settings, roi) retrieves a tile from the image with
     SETTINGS applied. The ROI specifies which pixels of the final
     image to return.
     This fails (returning a null image) if we do not have the full
     resolution image.
  */
  Image16 retrieveReducedROI(Adjustments const &settings,
                             QRect roi, PSize maxSize);
  /* RETRIEVEREDUCEDROI - Retrieves a tile from the image with settings applied.
     RETRIEVEREDUCEDROI(settings, roi, maxSize) retrieves a tile from the
     image with SETTINGS applied and reduced
     in resolution to fit within the given MAXSIZE.
     ROI is specified in units of pixels of the *unreduced* final image.
     This always succeeds, even if we don't have sufficient resolution
     to fill MAXSIZE. (Notes for RETRIEVEREDUCED apply.)
  */

  void enableCaching(bool ec=true);
  /* ENABLECACHING - Enable storing intermediate stages.
     ENABLECACHING() enables storing intermediate stages.
     ENABLECACHING(ec) enables (if EC is true) or disables caching.
     When caching is enabled, it is much faster to reprocess images
     after some parameter changes, but it obviously takes more memory,
     so should be avoided during export.  Default is enabled.
  */
  void disableCaching();
  // DISABLECACHING - Disable storing intermediate stages.
  bool isCaching() const { return caching; }
  // ISCACHING - Report whether caching is currently enabled
  void preserveOriginal(bool po=true);
  /* PRESERVEORIGINAL - Enable unconditional preservation of original image
     PRESERVEORIGINAL() enables preserving of the original image when caching
     is otherwise disabled.
     PRESERVEORIGINAL(po) enables (if PO is true) or disables this feature.
     Default is enabled.
     When disabled and caching is also
     disabled, calls to the retrieveXXX functions likely remove the original
     image from the adjuster, meaning that subsection retrieveXXX calls will
     fail.
     Technical note: This feature prevents me from declaring the retrieveXXX
     functions const and the stages mutable. Oh well.
  */
  bool preservesOriginal() const { return keeporiginal; }
  // PRESERVESORIGINAL - Report whether original preservation is enabled
  void cancel();
  /* CANCEL - Attempt to abort retrieve operations
     Adjuster is not truly threadsafe, but one exception exists: It is
     always safe to call CANCEL(), which will attempt to abort any
     "retrieve" operation and cause it to return an empty image. There
     is, however, no guarantee that cancellation will be
     effective. CANCEL() will return immediately and not wait to see
     if it was successful.
   */
private:
  bool isCanceled();
  /* Test-and-reset the cancellation flag. */
  void resetCanceled();
  bool applyUMask(Adjustments const &settings);
  bool applyEqualize(Adjustments const &settings);
  bool applyFirstXYZ(Adjustments const &settings);
  bool applyIPT(Adjustments const &settings);
  /* Apply those settings that work on a per-pixel basis from the given
     settings. This assumes that topmost stage exists and is a suitable
     basis for the requested settings. Intermediate stages may be stored
     if caching is enabled; the previous topmost stage may be removed if not.
     The original image is never removed if preserveOriginal is set.
     Returns true if it could be done.
   */
  bool applyGeometry(Adjustments const &settings);
  bool ensureAlreadyGood(class AdjusterStage const &adj, int iparent,
			 Adjustments const &final);
  int findParentStage(Stage s) const;
  void dropFrom(int k);
  bool applySettings(Adjustments const &settings);
  void applyNeedBasedScaling(Adjustments const &settings, PSize desired);
private:
  QList<AdjusterTile> stages;
  /* The first stage is always the original image; subsequent stages may
     be used to cache various processing stages to speed up reprocessing.
  */
  bool caching, keeporiginal, canceled;
  int maxthreads;
};

#endif
