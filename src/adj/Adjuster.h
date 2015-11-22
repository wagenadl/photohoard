// Adjuster.h

#ifndef ADJUSTER_H

#define ADJUSTER_H

#include "Image16.h"
#include "Sliders.h"
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
     versions of a photo
   */
  Q_OBJECT;
public:
  // Constructor: Adjuster
  Adjuster(QObject *parent=0);
  virtual ~Adjuster();

  void clear();
  // CLEAR - Drop entire stack
  bool isEmpty() const;
  // ISEMPTY - Return true if no image is set
  void setOriginal(Image16 const &image);
  /* SETORIGINAL - Loads a new original image into the adjuster
     SETORIGINAL(image) loads the given IMAGE into the adjuster, dropping
     any stages that previously existed. */
  void setReduced(Image16 const &image, PSize originalSize);
  /* SETREDUCED - Loads a new original image into the adjuster at less than full size
     SETREDUCED(image, originalSize) loads a new original image into the
     adjuster, just like SETORIGINAL, but not at its full
     resolution.
     The image may be at any resolution (smaller than the
     original); the original size must be specified through
     ORIGINALSIZE. SETREDUCED determines the scale
     of the reduced image from the actual and original sizes, taking the
     average of the X and Y scale factors.
  */
  Image16 retrieveFull(Sliders const &settings);
  /* RETRIEVEFULL - Retrieves a version of the image with settings applied.
     RETRIEVEFULL(settings) retrieves an image, at full resolution, to which
     all the given SETTINGS have been applied.
     RETRIEVEFULL fails (returning a null image) if we do not have the
     full resolution.
     Results are cached so that asking for the same image twice is fast.
  */
  Image16 retrieveReduced(Sliders const &settings, PSize maxSize);
  /* RETRIEVEREDUCED - As RETRIEVEFULL, but does not need full res.
     RETRIEVEREDUCED(settings, maxSize) retrieves a version of the
     image with SETTINGS applied and (usually)
     reduced in resolution to fit within the given MAXSIZE.
     This always succeeds, even if we don't have enough resolution to give
     the requested size. (The best available size is returned in that case.)
     Note that a larger than requested image may be returned if that is
     quicker. RETRIEVEREDUCED will never upscale an image.
  */

  PSize maxAvailableSize(Sliders const &settings) const;
  /* MAXAVAILABLESIZE - Maximum size of an image that we can offer
     MAXAVAILABLESIZE(settings) returns the maximum size of an image
     that we have on offer for the given SETTINGS. The image itself must
     be obtained through RETRIEVEREDUCED. Note that the max available size
     depends both on the size of the original (as per SETREDUCED) and on
     the SETTINGS. (Especially, of course, crop and rotation settings.)
  */     
  static PSize mapCropSize(PSize osize, Sliders const &settings,
                           PSize scaledOSize);
  /* MAPCROPSIZE - Determine size of cropped and scaled image
     MAPCROPSIZE(osize, settings, scaledOSize) calculates the size of a
     cropped version of an original image of size OSIZE if SETTINGS would
     be applied to it and if a scaling would be applied that would reduce
     the size of the original (uncropped) image to SCALEDOSIZE.
  */
  static QRect mapCropRect(PSize osize, Sliders const &settings,
                           PSize scaledOSize);
  /* MAPCROPRECT - Determine area of crop in scaled image
     MAPCROPRECT(osize, settings, scaledOSize) calculates the size and
     location of the cropping rectangle specified in SETTINGS if the image
     had been scaled from its original size OSIZE down to SCALEDOSIZE.
  */
  PSize neededScaledOriginalSize(Sliders const &settings, PSize desired) const;
  /* NEEDEDSCALEDORIGINALSIZE - Minimum size required to produce desired version
     NEEDEDSCALEDORIGINALSIZE(settings, desired) calculates how large
     a scaled version of an original image is needed to produce an
     adjusted version (as specified in SETTINGS) at the DESIRED final
     size. This applies to the currently loaded image.
   */
  static PSize neededScaledOriginalSize(PSize osize, Sliders const &settings,
                                        PSize desired);
  /* NEEDEDSCALEDORIGINALSIZE - Minimum size required to produce desired version
     NEEDEDSCALEDORIGINALSIZE(osize, settings, desired) calculates how
     large a scaled version of an original is needed to produce an
     adjusted version (as specified in SETTINGS) at the DESIRED final
     size. This static version does its calculations for an orinal image of
     full size OSIZE.
   */
  Image16 retrieveROI(Sliders const &settings, QRect roi);
  /* RETRIEVEROI - Retrieves a tile from the image with settings applied.
     RETRIEVEROI(settings, roi) retrieves a tile from the image with
     SETTINGS applied. The ROI specifies which pixels of the final
     image to return.
     This fails (returning a null image) if we do not have the full
     resolution image.
  */
  Image16 retrieveReducedROI(Sliders const &settings,
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
     ENABLECACHAING(ec) enables (if EC is true) or disables caching.
     When caching is enabled,
     it is much faster to reprocess images after some parameter changes,
     but it obviously takes more memory, so should be avoided during export.
     Default is enabled.
  */
  void disableCaching();
  // DISABLECACHING - Disable storing intermediate stages.
  bool isCaching() const { return caching; }
  // ISCACHING - Report whether caching is currently enabled
  void preserveOriginal(bool po=true);
  /* PRESERVEORIGINAL - Enable unconditional preservation of original image
     PRESERVEORIGINAL() enables preserving of the original image when caching is
     otherwise disabled.
     PRESERVEORIGINAL(po) enables (if PO is true) or disables this feature.
     Default is eanbled.
     When disabled and caching is also
     disabled, calls to the retrieveXXX functions likely remove the original
     image from the adjuster, meaning that subsection retrieveXXX calls will
     fail.
     Technical note: This feature prevents me from declaring the retrieveXXX
     functions const and the stages mutable. Oh well.
  */
  bool preservesOriginal() const { return keeporiginal; }
  // PRESERVESORIGINAL - Report whether original preservaion is enabled
  void cancel();
  /* CANCEL - Attempt to abort retrieve operations
     Adjuster is not truly threadsafe, but one exception exists: It is
     always safe to call CANCEL(), which will attempt to abort any
     "retrieve" operation and cause it to return an empty image. There is,
     however, no guarantee that cancellation will be effective. CANCEL() will
     return immediately and not wait to see if it was successful.
   */
private:
  bool isCanceled();
  /* Test-and-reset the cancellation flag. */
  void resetCanceled();
  bool applyUMask(Sliders const &settings);
  bool applyEqualize(Sliders const &settings);
  bool applyFirstXYZ(Sliders const &settings);
  bool applyIPT(Sliders const &settings);
  /* Apply those settings that work on a per-pixel basis from the given
     settings. This assumes that topmost stage exists and is a suitable
     basis for the requested settings. Intermediate stages may be stored
     if caching is enabled; the previous topmost stage may be removed if not.
     The original image is never removed if preserveOriginal is set.
     Returns true if it could be done.
   */
  bool applyGeometry(Sliders const &settings);
  bool ensureAlreadyGood(class AdjusterStage const &adj, int iparent,
			 Sliders const &final);
  int findParentStage(Stage s) const;
  void dropFrom(int k);
  bool applySettings(Sliders const &settings);
  void applyNeedBasedScaling(Sliders const &settings, PSize desired);
private:
  QList<AdjusterTile> stages;
  /* The first stage is always the original image; subsequent stages may
     be used to cache various processing stages to speed up reprocessing.
  */
  bool caching, keeporiginal, canceled;
};

#endif
