// AdjusterGeometry.cpp

#include "AdjusterGeometry.h"
#include "Adjuster.h"
#include "Geometry.h"
#include <math.h>

QStringList AdjusterGeometry::fields() const {
  static QStringList flds
    = QString("rotate cropl cropr cropt cropb perspv persph shearv shearh").
    split(" ");
  return flds;
}
  
AdjusterTile AdjusterGeometry::apply(AdjusterTile const &parent,
				Adjustments const &fin) {
  AdjusterTile tile = parent;
  
  tile.stage = Stage_Geometry;
  tile.image.convertTo(Image16::Format::LMS16, maxthreads);

  // ROTATE
  if (fin.rotate)
    tile.image = tile.image.rotated(-M_PI*fin.rotate/180);

  // PERSPECTIVE
  if (Geometry::hasPerspectiveTransform(fin)) {
    tile.image
      = Geometry::perspectiveTransform(tile.image.size(), fin)
      .warp(tile.image);
  }
  
  // CROP
  if (fin.cropl || fin.cropr || fin.cropt || fin.cropb) {
    tile.image.crop(Geometry::scaledCropRect(parent.osize, fin,
                                             parent.image.size()));
    if (tile.image.isNull()) {
      tile.image = Image16(QSize(1,1));
      uchar *d = tile.image.bytes();
      d[0] = 128;
      d[1] = 128;
      d[2] = 128;
      d[3] = 255;
    }
  }

  tile.settings.rotate = fin.rotate;
  tile.settings.cropl = fin.cropl;
  tile.settings.cropr = fin.cropr;
  tile.settings.cropt = fin.cropt;
  tile.settings.cropb = fin.cropb;
  tile.settings.perspv = fin.perspv;
  tile.settings.persph = fin.persph;
  tile.settings.shearv = fin.shearv;
  tile.settings.shearh = fin.shearh;

  return tile;
}
