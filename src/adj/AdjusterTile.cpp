// AdjusterTile.cpp

#include "AdjusterTile.h"
#include "PDebug.h"

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
  double factor = tile.image.size() / image.size();
  tile.nominalScale *= factor;
  if (tile.stage < Stage_Reduced)
    tile.stage = Stage_Reduced;
  return tile;
}

AdjusterTile AdjusterTile::cutROI(QRect roi) const {
  AdjusterTile tile = *this;
  if (isROI)
    COMPLAIN("cutROI on already cut images is not yet supported");
  tile.image.crop(roi);
  tile.roiOffset = roi.topLeft();
  tile.isROI = true;
  return tile;
}

AdjusterTile AdjusterTile::cropped(Adjustments const &crop) const {
  if (isROI) 
    COMPLAIN("It is an error to crop after pulling an ROI");
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
  tile.roiScale = nominalScale;
  return tile;
}
