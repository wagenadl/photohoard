// AdjusterTile.h

#ifndef ADJUSTERTILE_H

#define ADJUSTERTILE_H

#include "Image16.h"
#include "Adjustments.h"
#include <QRect>
#include "Stage.h"

class AdjusterTile {
public:
  AdjusterTile();
  explicit AdjusterTile(Image16 const &);
  explicit AdjusterTile(Image16 const &, PSize osize);
public:
  Image16 image;
  Adjustments settings;
  QRect roi; // specified in units of the original image or empty for no-roi
  PSize osize;
  Stage stage;
  /* I think we should be smarter about ROIs. We should store also where
     in the scaled image the ROI lives, and have a PSize for the full size
     of the scaled image. (The size of the ROI in the scaled image is
     evident from the image, of course.) I think an isROI flag is better
     than using the QRect. In fact, there may not be much point in storing
     the position of the ROI in the original image at all.
     This "ssize", "offset", "isROI" are better variables than "roi". (Note
     that isROI is trivially computed from image.size and ssize, but we might
     as well store it explicitly.)
     I am not sure how I feel about all these fields being public. Who sets
     them? Surely only the creator. No. In fact, the AdjusterXX mark up the
     settings field directly.
  */
};

#endif
