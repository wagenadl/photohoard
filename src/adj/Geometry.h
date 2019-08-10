// Geometry.h

#ifndef GEOMETRY_H

#define GEOMETRY_H

#include "PSize.h"
#include "Adjustments.h"
#include "PerspectiveTransform.h"

namespace Geometry {
  PSize croppedSize(PSize osize, Adjustments const &settings);
  /* CROPPEDSIZE - Determine size of cropped image
     CROPPEDSIZE(osize, settings) returns the size of the crop rectangle
     if the parameters in SETTINGS are applied to an image of original
     size OSIZE.
  */
  PSize scaledCroppedSize(PSize osize, Adjustments const &settings,
                                 PSize scaledOSize);
  /* SCALEDCROPPEDSIZE - Determine size of cropped and scaled image
     SCALEDCROPPEDSIZE(osize, settings, scaledOSize) returns the scaled
     size of the crop rectangle if the parameters in SETTINGS are applied to
     an image of original size OSIZE that is scaled to SCALEDOSIZE (before
     cropping).
  */
  QRect cropRect(PSize osize, Adjustments const &settings);
  /* SCALEDCROPRECT - Determine crop rectangle in scaled image
     SCALEDCROPRECT(osize, settings) calculates the size and
     location of the cropping rectangle specified in SETTINGS if those
     settings are applied to an image with original size OSIZE.
  */
  QRect scaledCropRect(PSize osize, Adjustments const &settings,
                              PSize scaledOSize);
  /* SCALEDCROPRECT - Determine crop rectangle in scaled image
     SCALEDCROPRECT(osize, settings, scaledOSize) calculates the size and
     location of the cropping rectangle specified in SETTINGS if the image
     had been scaled from its original size OSIZE down to SCALEDOSIZE.
  */
  PSize neededScaledOriginalSize(PSize osize,
                                 Adjustments const &settings,
                                 PSize desired);
  /* NEEDEDSCALEDORIGINALSIZE - Minimum size required to produce desired version
     NEEDEDSCALEDORIGINALSIZE(osize, settings, desired) calculates how
     large a scaled version of an original is needed to produce an adjusted
     version (as specified in SETTINGS) at the DESIRED final size.
     Calculations asume an orinal image of full size OSIZE.
   */
  QPointF mapToAdjusted(QPointF, QSize osize, Adjustments const &);
  QPolygonF mapToAdjusted(QPolygonF, QSize osize, Adjustments const &);
  /* MAPTOADJUSTED - Maps a point in an original image to adjusted image
     MAPTOADJUSTED(p, osize, settings) maps a point P from an original image
     with size OSIZE to its final location if given SETTINGS are applied to
     the image.
  */
  QPointF mapToScaledAdjusted(QPointF, QSize osize, Adjustments const &,
                              QSize scaledOSize);
  QPolygonF mapToScaledAdjusted(QPolygonF, QSize osize, Adjustments const &,
                                QSize scaledOSize);
  QPointF mapToScaledAdjusted(QPointF, QSize osize, Adjustments const &,
                              double scale);
  QPolygonF mapToScaledAdjusted(QPolygonF, QSize osize, Adjustments const &,
                                double scale);
  /* MAPTOSCALEDADJUSTED - Maps a point in an original image to adjusted image
     MAPTOSCALEDADJUSTED(p, osize, settings, scaledosize) or
     MAPTOSCALEDADJUSTED(p, osize, settings, scalefactor) maps a point P to
     its final position if given SETTINGS are applied to an image of original
     size OSIZE that is scaled to SCALEDOSIZE (before cropping) or by
     SCALEFACTOR.
  */
  bool hasPerspectiveTransform(Adjustments const &);
  PerspectiveTransform perspectiveTransform(QSize osize, Adjustments const &);
};

#endif
