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
  AdjusterTile scaledToFitSnuglyIn(PSize) const;
  AdjusterTile cropped(Adjustments const &) const;
  // spec'd in terms of original pixels, even if we are scaled
  AdjusterTile cutROI(QRect) const;
  /* Note the conceptual difference between cutting an ROI for internal
     use vs. performing a crop because of the user's choice. */
public:
  Stage stage; // OK to modify
  Adjustments settings; // OK to modify
  Image16 image; // OK to modify, but do not resize
  double nominalScale; // read-only
  PSize osize; // pre-crop, pre-scaling, pre-ROI; read-only
  bool isROI; // read-only
  QPoint roiOffset; // watch out for units!; read-only
  double roiScale; // nominal scale of roiOffset; read-only
  /* The roiOffset may be in original units or scaled units or something
     intermediate. Check roiScale for details! (Often, roiScale will be
     either 1 or the same as nominalScale, but it can be different too.)
  */
};

#endif
