// AdjusterGeometry.cpp

#include "AdjusterGeometry.h"
#include "Adjuster.h"

QStringList AdjusterGeometry::fields() const {
  static QStringList flds
    = QString("rotate cropl cropr cropt cropb perspv persph shearv shearh").
    split(" ");
  return flds;
}

QPointF AdjusterGeometry::map(QPointF p, QSize osize, Adjustments const &adj) {
  double w = osize.width();
  double h = osize.height();

  if (adj.rotate) {
    double phi = -M_PI*adj.rotate/180;
    QPointF pc = p - QPointF(w/2, h/2);
    pc = QPointF(pc.x()*cos(phi) - pc.y()*sin(phi),
		 pc.x()*sin(phi) + pc.y()*cos(phi));
    p = pc + QPointF(w/2, h/2);
  }

  if (adj.perspv || adj.persph || adj.shearv || adj.shearh) {
    double dx = 2 * p.x() / w - 1;
    double dy = 2 * p.y() / h - 1;
    p += QPointF(w*(dx*dy*adj.perspv + dy*adj.shearh),
		 h*(-dx*dy*adj.persph + dx*adj.shearv));
    // is this correct?
  }

  if (adj.cropl || adj.cropt) 
    p -= QPointF(adj.cropl, adj.cropt);

  return p;
}
  
AdjusterTile AdjusterGeometry::apply(AdjusterTile const &parent,
				Adjustments const &final) {
  AdjusterTile tile = parent;
  
  tile.stage = Stage_Geometry;
  tile.image.convertTo(Image16::Format::LMS16);

  // ROTATE
  if (final.rotate)
    tile.image = tile.image.rotated(-M_PI*final.rotate/180);

  // PERSPECTIVE
  if (final.perspv || final.persph || final.shearv || final.shearh) {
    QPolygonF pp;
    double w = tile.image.width();
    double h = tile.image.height();
    // top left
    pp << QPointF(w*(-final.perspv-final.shearh)/100.0,
                  h*(+final.persph-final.shearv)/100.0);
    // top right
    pp << QPointF(w*(100+final.perspv-final.shearh)/100.0,
                  h*(-final.persph+final.shearv)/100.0);
    // bottom left
    pp << QPointF(w*(final.perspv+final.shearh)/100.0,
                  h*(100-final.persph-final.shearv)/100.0);
    // bottom right
    pp << QPointF(w*(100-final.perspv+final.shearh)/100.0,
                  h*(100+final.persph+final.shearv)/100.0);
    tile.image = tile.image.perspectived(pp);
  }
  
  // CROP
  if (final.cropl || final.cropr || final.cropt || final.cropb) {
    tile.image.crop(Adjuster::mapCropRect(parent.osize, final,
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
