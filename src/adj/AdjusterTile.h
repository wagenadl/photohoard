// AdjusterTile.h

#ifndef ADJUSTERTILE_H

#define ADJUSTERTILE_H

#include "Image16.h"
#include "Sliders.h"
#include <QRect>
#include "Stage.h"

class AdjusterTile {
public:
  AdjusterTile();
  explicit AdjusterTile(Image16 const &);
  explicit AdjusterTile(Image16 const &, QSize osize);
public:
  Image16 image;
  Sliders settings;
  QRect roi; // specified in units of the original image
  double scale;
  Stage stage;
};

#endif