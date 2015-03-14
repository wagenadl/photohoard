// Adjuster.h

#ifndef ADJUSTER_H

#define ADJUSTER_H

#include "Image16.h"
#include "Sliders.h"
#include <QRect>
#include "PSize.h"
#include <QObject>
#include "AdjusterTile.h"

class Adjuster: public QObject {
  Q_OBJECT;
public:
  Adjuster(QObject *parent=0);
  virtual ~Adjuster();
  void clear();
  /* Drop entire stack */
  bool isEmpty() const;
  void setOriginal(Image16 const &image);
  /* Loads a new original image into the adjuster */
  void setReduced(Image16 const &image, PSize originalSize);
  /* Loads a new original image into the adjuster, but not at its full
     resolution. The image may be at any resolution (smaller than the
     original); the original size is specified. We determine the scale
     of the reduced image from the actual and original sizes, taking the
     average of the X and Y scale factors.
  */
  Image16 retrieveFull(Sliders const &settings);
  /* Retrieves a version of the image with settings applied. This fails
     (returning a null image) if we do not have the full resolution.
  */
  Image16 retrieveReduced(Sliders const &settings, PSize maxSize);
  /* Retrieves a version of the image with settings applied and possibly
     reduced in resolution to fit within the given maxSize.
     This always succeeds, even if we don't have enough resolution to give
     the requested size. (A further reduced version is returned in that case.)
     Note that a _larger_ image may also be returned if that is quicker.
  */
  PSize maxAvailableSize(Sliders const &settings) const;
  static PSize mapCropSize(PSize osize, Sliders const &settings,
                           PSize scaledOSize);
  static QRect mapCropRect(PSize osize, Sliders const &settings,
                           PSize scaledOSize);
  /* Maps a crop rectangle specified in original image image coordinates
     to scaled image coordinates. */
  PSize neededScaledOriginalSize(Sliders const &settings, PSize desired) const;
  static PSize neededScaledOriginalSize(PSize osize, Sliders const &settings,
                                        PSize desired);
  /* Given a desired output size, calculate how large the original image
     must be to get that after cropping. */
  Image16 retrieveROI(Sliders const &settings, QRect roi);
  /* Retrieves a tile from the image with settings applied. The ROI
     specifies which pixels of the final image to return. This fails
     (returning a null image) if we do not have the full resolution image.
  */
  Image16 retrieveReducedROI(Sliders const &settings,
                             QRect roi, PSize maxSize);
  /* Retrieves a tile from the image with settings applied and reduced
     in resolution to fit within the given maxSize.
     ROI is specified in units of pixels of the *unreduced* final image.
     This always succeeds, even if we don't have sufficient resolution
     to fill maxSize. (A further reduced version is returned in that
     case.)
  */
  void enableCaching(bool ec=true);
  void disableCaching();
  bool isCaching() const { return caching; }
  /* Enables or disables the storing of intermediate stages. When enabled,
     it is much faster to reprocess images after some parameter changes,
     but it obviously takes more memory, so should be avoided during export.
     Default is enabled. */
  void preserveOriginal(bool po=true);
  bool preservesOriginal() const { return keeporiginal; }
  /* Enables or disables preserving of the original image when caching is
     otherwise disabled. Default is true. When set to false and caching is
     disabled, calls to the retrieveXXX functions likely remove the original
     image from the adjuster, meaning that subsection retrieveXXX calls will
     fail.
     Note that this feature prevents me from declaring the retrieveXXX
     functions const and the stages mutable. Oh well.
  */
  void cancel();
  /* Adjuster is not truly threadsafe, but one exception exists: It is
     always safe to call cancel(), which will attempt to abort any
     "retrieve" operation and cause it to return an empty image. There is,
     however, no guarantee that cancellation will be effective.
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
