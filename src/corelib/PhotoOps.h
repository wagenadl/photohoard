// PhotoOps.h

#ifndef PHOTOOPS_H

#define PHOTOOPS_H

#include "Image16.h"

namespace PhotoOps {
  Image16 seamlessClone(Image16 const &target,
                        Image16 const &source, QImage const &mask,
                        QPoint p, int method);
  /* Places the SOURCE image over the TARGET image at point P using
     OpenCV's "seamless cloning" technique (from Perez et al., 2003).
     MASK must be an 8-bit monochrome image with the same size as SOURCE.
     METHOD is 1 for normal, 2 for mixed, 3 for monochrome.
     See https://docs.opencv.org/3.4/df/da0/group__photo__clone.html.
   */
  Image16 inpaint(Image16 const &target,
                  QImage const &mask,
                  double radius,
                  int method);
  /* Replaces pixels in the TARGET image that are "on" in the MASK (which
     must be the same size as the target) with reasonable estimates from
     the neighborhood. RADIUS is specified in pixels.
     METHOD is 1 for Navier-Stokes, or 2 for Telea at all.
     See https://docs.opencv.org/3.4/d7/d8b/group__photo__inpaint.html. */

  Image16 decolorizeOrBoost(Image16 const &target, bool boost=false);
  void blur(QImage &target, double sigma);
};

#endif
