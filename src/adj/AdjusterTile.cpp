// AdjusterTile.cpp

#include "AdjusterTile.h"

AdjusterTile::AdjusterTile() {
  stage = Stage_Original;
  isROI = false;
  nominalScale = 1;
}

AdjusterTile::AdjusterTile(Image16 const &img): image(img) {
  osize = img.size();
  stage = Stage_Original;
  isROI = false;
  nominalScale = 1;
}

AdjusterTile::AdjusterTile(Image16 const &img, PSize osize):
  image(img), osize(osize) {
  stage = Stage_Reduced;
  isROI = false;
  nominalScale = img.size() / osize;
}

AdjusterTile AdjusterTile::scaledToFitSnuglyIn(PSize s) const {
  AdjusterTile tile = *this;
  tile.image = image.scaledToFitSnuglyIn(s);
  tile.nominalScale *= tile.image.size() / image.size();
  if (tile.stage < Stage_Reduced)
    tile.stage = Stage_Reduced;
  return tile;
}

AdjusterTile AdjusterTile::cutROI(QRect roi) const {
  AdjusterTile tile = *this;
  tile.image.crop(roi);
  tile.roiOffset = roi.topLeft();
  tile.isROI = true;
  return tile;
}

AdjusterTile AdjusterTile::cropped(Adjustments const &crop) const {
  AdjusterTile tile = *this;
  if (nominalScale==1) {
    QRect rect(QPoint(crop.cropl, crop.cropt),
               QSize(osize.width()-crop.cropl-crop.cropr,
                     osize.height()-crop.cropt-crop.cropb));
    tile.image.crop(rect);
  } else {
    QRect rect(QPoint(crop.cropl*nominalScale + .5,
                      crop.cropt*nominalScale + .5),
               QSize((osize.width()-crop.cropl-crop.cropr)*nominalScale + .5,
                     (osize.height()-crop.cropt-crop.cropb)*nominalScale + .5));
    tile.image.crop(rect);
  }    
  return tile;
}
