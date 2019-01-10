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
public:
  Stage stage; // OK to modify
  Adjustments settings; // OK to modify
  Image16 image; // OK to modify, but do not resize
  double nominalScale; // read-only
  PSize osize; // read-only
  QPoint roiOffset; // in scaled image; read-only
  bool isROI; // read-only
};

#endif
