// AdjusterTile.cpp

#include "AdjusterTile.h"

AdjusterTile::AdjusterTile() {
  scale = 1;
  stage = Stage_Original;
}

AdjusterTile::AdjusterTile(Image16 const &img) {
  image = img;
  roi = QRect(QPoint(0, 0), img.size());
  scale = 1;
  stage = Stage_Original;
}

AdjusterTile::AdjusterTile(Image16 const &img, QSize osize) {
  image = img;
  double xsc = img.width() / double(osize.width());
  double ysc = img.height() / double(osize.height());
  roi = QRect(QPoint(0, 0), osize);
  scale = sqrt(xsc*ysc);
  stage = Stage_Scaled;
}


