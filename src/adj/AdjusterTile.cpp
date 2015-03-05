// AdjusterTile.cpp

#include "AdjusterTile.h"

AdjusterTile::AdjusterTile() {
  stage = Stage_Original;
}

AdjusterTile::AdjusterTile(Image16 const &img): image(img) {
  osize = img.size();
  stage = Stage_Original;
}

AdjusterTile::AdjusterTile(Image16 const &img, QSize osize):
  image(img), osize(osize) {
  stage = Stage_Reduced;
}
