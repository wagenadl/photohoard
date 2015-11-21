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
  Q_OBJECT;
public:
  // Constructor: Adjuster
  Adjuster(QObject *parent=0);
  virtual ~Adjuster();

  // Function: clear
  // Drop entire stack of partially adjusted images
  void clear();

  // Function: isEmpty
  // Returns true if we don't have an image loaded
  bool isEmpty() const;

  /* Function: setOriginal
     Loads a new original image into the adjuster
  */
  void setOriginal(Image16 const &image);

  /* function: setReduced

     Loads a new original image into the adjuster, but not at its full
     resolution. The image may be at any resolution (smaller than the
     original); the original size is specified. We determine the scale
     of the reduced image from the actual and original sizes, taking the
     average of the X and Y scale factors.
  */
  void setReduced(Image16 const &image, PSize originalSize);
  
  /* Function: retrieveFull

     Retrieves a version of the image with settings applied. This fails
     (returning a null image) if we do not have the full resolution.
  */
  Image16 retrieveFull(Sliders const &settings);

  /* Function: retrieveReduced

     Retrieves a version of the image with settings applied and possibly
     reduced in resolution to fit within the given maxSize.
     This always succeeds, even if we don't have enough resolution to give
     the requested size. (A further reduced version is returned in that case.)
     Note that a _larger_ image may also be returned if that is quicker.
  */
  Image16 retrieveReduced(Sliders const &settings, PSize maxSize);

  // Function: maxAvailableSize
  // Returns the largest size that we can produce for the given settings,
  // given what we have been given by <setReduced> or <setOriginal>.
  PSize maxAvailableSize(Sliders const &settings) const;

  /* Function: mapCropSize
     Returns the size of the cropping rectange specified in settings after
     scalling the full image from osize down to scaledOSize.
   */
  static PSize mapCropSize(PSize osize, Sliders const &settings,
                           PSize scaledOSize);

  /* Function: mapCropRect
     If an image is scaled from osize to scaledOSize, the cropping rectangle
     specified in settings is transformed. This function calculates that
     transform.
   */
  static QRect mapCropRect(PSize osize, Sliders const &settings,
                           PSize scaledOSize);

  /* Function: neededScaledOriginalSize
     Given the desired size of an output image, return at which scale the
     original needs to be loaded given a set of adjustments.
     */
  PSize neededScaledOriginalSize(Sliders const &settings, PSize desired) const;

  // Function: neededScaledOriginalSize
  // Given a desired output size, calculate how large the original image
  // must be to get that after cropping. 
  static PSize neededScaledOriginalSize(PSize osize, Sliders const &settings,
                                        PSize desired);
  
  /* Function: retrieveROI

     Retrieves a tile from the image with settings applied. The ROI
     specifies which pixels of the final image to return. This fails
     (returning a null image) if we do not have the full resolution image.
  */
  Image16 retrieveROI(Sliders const &settings, QRect roi);

  /* Function: retrieveReducedROI

     Retrieves a tile from the image with settings applied and reduced
     in resolution to fit within the given maxSize.
     ROI is specified in units of pixels of the *unreduced* final image.
     This always succeeds, even if we don't have sufficient resolution
     to fill maxSize. (A further reduced version is returned in that
     case.)
  */
  Image16 retrieveReducedROI(Sliders const &settings,
                             QRect roi, PSize maxSize);

  // Function: enableCaching
  void enableCaching(bool ec=true);

  // Function: disableCaching
  void disableCaching();

  /* Function: isCaching
     Returns whether caching is enabled.

     When enabled, it is much faster to reprocess images after some
     parameter changes, but it obviously takes more memory, so should
     be avoided during export.  Default is enabled. */
  bool isCaching() const { return caching; }

  // Function: preserveOriginal
  void preserveOriginal(bool po=true);

  /* Function: preservesOriginal
  /* Returns whether original images are preserved when caching is
     otherwise disabled.

     Default is true. When set to false and caching is
     disabled, calls to the retrieveXXX functions likely remove the original
     image from the adjuster, meaning that subsection retrieveXXX calls will
     fail.

     Note that this feature prevents me from declaring the retrieveXXX
     functions const and the stages mutable. Oh well.
  */
  bool preservesOriginal() const { return keeporiginal; }
  
  /* Function: cancel

     Adjuster is not truly threadsafe, but one exception exists: It is
     always safe to call cancel(), which will attempt to abort any
     "retrieve" operation and cause it to return an empty image. There is,
     however, no guarantee that cancellation will be effective.
   */
  void cancel();
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
