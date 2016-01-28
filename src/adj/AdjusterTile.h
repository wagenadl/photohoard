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
};

#endif
