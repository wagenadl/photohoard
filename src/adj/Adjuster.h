// Adjuster.h

#ifndef ADJUSTER_H

#define ADJUSTER_H

#include "Image16.h"
#include "Sliders.h"
#include <QRect>
#include <QSize>
#include <QObject>

class Adjuster: public QObject {
  Q_OBJECT;
public:
  Adjuster(QObject *parent=0);
  virtual ~Adjuster();
  void setOriginal(Image16 const &image);
  /* Loads a new original image into the adjuster */
  void setReduced(Image16 const &image, QSize originalSize);
  /* Loads a new original image into the adjuster, but not at its full
     resolution. The image may be at any resolution (smaller than the
     original); the original size is specified. We determine the scale
     of the reduced image from the actual and original sizes, taking the
     average of the X and Y scale factors.
  */
  QSize maxAvailableSize() const;
  double maxAvailableScale() const;
  /* Returns the maximum available size or scale of the image. */
  Image16 retrieveFull(Sliders const &settings);
  /* Retrieves a version of the image with settings applied. This fails
     (returning a null image) if we do not have the full resolution.
  */
  Image16 retrieveReduced(Sliders const &settings, QSize maxSize);
  /* Retrieves a version of the image with settings applied and reduced
     in resolution to fit within the given maxSize. This always succeeds,
     even if we don't have enough resolution to give the requested size.
     (A further reduced version is returned in that case.)  */
  QSize finalSize(Sliders const &settings) const;
  /* Determine the size of the final image. This may differ from the size
     of the original image due to cropping or other geometric transforms.
  */
  Image16 retrieveROI(Sliders const &settings, QRect roi);
  /* Retrieves a tile from the image with settings applied. The ROI
     specifies which pixels of the final image to return. This fails
     (returning a null image) if we do not have the full resolution image.
  */
  Image16 retrieveReducedROI(Sliders const &settings,
                             QRect roi, QSize maxSize);
  /* Retrieves a tile from the image with settings applied and reduced
     in resolution to fit within the given maxSize.
     ROI is specified in units of pixels of the *unreduced* final image.
     This always succeeds, even if we don't have sufficient resolution
     to fill maxSize. (A further reduced version is returned in that
     case.)
  */
  double estimateScale(Sliders const &settings, QSize imageSize);
  /* Estimates the scale of a retrieved image given its size. */
  double estimateScaleForROI(Sliders const &settings,
                             QRect roi, QSize imageSize);
  /* As estimateScale, but for a final image that represents a ROI of the
     original image. ROI should be as given to retrieveReducedROI when
     obtaining the respective image. */
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
private:
  void dropIncompatibleStages(Sliders const &settings);
  /* Drop stages that cannot be ancestors of given settings.
     Note that this operates slider by slider and does not know about
     sliders that can only operate in pairs.
  */
  void applySinglePixelSettings(Sliders const &settings);
  /* Apply those settings that work on a per-pixel basis from the given
     settings. This assumes that topmost stage exists and is a suitable
     basis for the requested settings. Intermediate stages may be stored
     if caching is enabled; the previous topmost stage may be removed if not.
     The original image is never removed if preserveOriginal is set.
   */
  void applyBlackPointAndExpose(Sliders const &settings);
  // part of applySignelPixelSettings
private:
  class AdjustedTile {
  public:
    AdjustedTile();
    explicit AdjustedTile(Image16 const &);
    explicit AdjustedTile(Image16 const &, QSize osize);
  public:
    Image16 image;
    Sliders settings;
    QRect roi; // specified in units of the original image
    double scale;
  };
  QList<AdjustedTile> stages;
  /* The first stage is always the original image; subsequent stages may
     be used to cache various processing stages to speed up reprocessing.
  */
  bool caching, keeporiginal;
};

#endif