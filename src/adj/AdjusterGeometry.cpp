// AdjusterGeometry.cpp

#include "AdjusterGeometry.h"
#include "Adjuster.h"

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

  QRect crop1 = Adjuster::mapCropRect(parent.osize, final,
				      parent.image.size());
  qDebug() << "AdjusterGeometry:"
	   << parent.osize << parent.image.size() << crop1;
  tile.image.crop(crop1);
  if (tile.image.isNull())
    tile.image = Image16(QSize(1,1));

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
