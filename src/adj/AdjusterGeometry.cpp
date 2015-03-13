// AdjusterGeometry.cpp

#include "AdjusterGeometry.h"

QStringList AdjusterGeometry::fields() const {
  static QStringList flds
    = QString("rotate cropl cropr cropt cropb perspv persph shearv shearh").
    split(" ");
  return flds;
}

AdjusterTile AdjusterGeometry::apply(AdjusterTile const &parent,
				Sliders const &final) {
  AdjusterTile tile = parent;
  
  tile.stage = Stage_Geometry;
  tile.image.convertTo(Image16::Format::LMS16);

  double angle = final.rotate;

  tile.image = tile.image.rotated(-M_PI*angle/180);

  tile.settings.rotate = final.rotate;
  tile.settings.cropl = final.cropl;
  tile.settings.cropr = final.cropr;
  tile.settings.cropt = final.cropt;
  tile.settings.cropb = final.cropb;
  tile.settings.perspv = final.perspv;
  tile.settings.persph = final.persph;
  tile.settings.shearv = final.shearv;
  tile.settings.shearh = final.shearh;

  return tile;
}
