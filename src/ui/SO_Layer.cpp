// SO_Layer.cpp

#include "SO_Layer.h"
#include <QPainter>
#include "AdjusterGeometry.h"
#include "PDebug.h"
#include <QMouseEvent>
#include "PhotoDB.h"
#include "Geometry.h"
#include "Spline.h"
#include <cmath>

static inline double L2norm(QPointF p) {
  double x = p.x();
  double y = p.y();
  return x*x + y*y;
}

static inline double euclideanLength(QPointF p) {
  return std::sqrt(L2norm(p));
}

class LayerGeomBase {
public:
  LayerGeomBase(SO_Layer const *so) {
    QPolygonF poly = so->layer.points();
    if (poly.isEmpty())
      return;

    QPointF p0;
    for (auto &p: poly)
      p0 += p;
    originalCenter = p0 / poly.size();

    QTransform const &xf = so->base()->transformationFromImage();
    transformedCenter = xf.map(Geometry::mapToAdjusted(originalCenter,
                                                       so->osize, so->adj));

    for (auto &p: poly)
      p = xf.map(Geometry::mapToAdjusted(p, so->osize, so->adj));
    transformedNodes = poly;

    for (double a: so->layer.radii()) {
      QPointF p = originalCenter + QPointF(a, 0);
      QPointF q = xf.map(Geometry::mapToAdjusted(p, so->osize, so->adj));
      transformedRadii << euclideanLength(q - transformedCenter);
    }
    if (transformedRadii.size())
      radiusFactor = so->layer.radii()[0] / (transformedRadii[0] + 1e-15);
    else
      radiusFactor = 1;
  }
public:
  QPolygonF transformedNodes;
  QList<double> transformedRadii;
  QPointF originalCenter;
  QPointF transformedCenter;
  double radiusFactor; // factor to multiply transformedRadii back to image
};
  

class ShapeLayerGeom {
public:
  ShapeLayerGeom(LayerGeomBase const &base) {
    if (base.transformedNodes.isEmpty())
      return;
    transformedCurve = Spline::catmullRom(base.transformedNodes, 2);
    int idx = (transformedCurve.origidx[0] + transformedCurve.origidx[1])/2;
    radiusAnchor = transformedCurve.points[idx];

    int N = transformedCurve.points.size();
    int n1 = idx + 5;
    if (n1>=N)
      n1 -= N;
    int n2 = idx - 5;
    if (n2<0)
      n2 += N;
    radiusAnchor1 = transformedCurve.points[n1];
    radiusAnchor2 = transformedCurve.points[n2];
    QPointF dir(radiusAnchor1  - radiusAnchor2);
    dir /= euclideanLength(dir);
    radiusNode = QPointF(radiusAnchor
                         + QPointF(-dir.y(), dir.x())
                           * base.transformedRadii[0]);
  }
public:
  Spline transformedCurve;
  QPointF radiusAnchor; // on transformedCurve
  QPointF radiusNode; // at a distance
  QPointF radiusAnchor1, radiusAnchor2;
};

SO_Layer::SO_Layer(PhotoDB *db, SlideView *sv): SlideOverlay(sv), db(db) {
  clickidx = -1;
}

void SO_Layer::setLayer(quint64 vsnid1, int lay1) {
  vsn = vsnid1;
  lay = lay1;
  layer = Layers(vsn, db).layer(lay);
  osize = db->originalSize(vsn);
  updateTransform();
}

void SO_Layer::updateTransform() {
  adj = Adjustments::fromDB(vsn, *db);
}

void SO_Layer::paintEvent(QPaintEvent *) {
  LayerGeomBase geom(this);

  switch (layer.type()) {
  case Layer::Type::LinearGradient:
    paintLinear(geom);
    break;
  case Layer::Type::Area:
    paintArea(geom);
    break;
  case Layer::Type::Clone:
    paintClone(geom);
    break;
  default:
    pDebug() << "Paint Layer type" << int(layer.type()) << "NYI";
    break;
  }
}

constexpr static int POINTRADIUS = 10;

void SO_Layer::paintLinear(LayerGeomBase const &geom) {
  QPainter ptr(this);
  bool first = true;
  QPen b(QColor(0,200,0));
  b.setWidth(3);
  ptr.setPen(b);
  for (auto const &p: geom.transformedNodes) {
    ptr.drawEllipse(p, POINTRADIUS, POINTRADIUS);
    if (first) {
      b.setColor(QColor(255,0,0));
      ptr.setPen(b);
      first = false;
    }
  }
}

void SO_Layer::paintCurve(LayerGeomBase const &geom) {
}

void SO_Layer::paintArea(LayerGeomBase const &geom) {
  ShapeLayerGeom shgeom(geom);
  
  QPainter ptr(this);
  QPen b(QColor(0,200,0));
  b.setWidth(3);
  ptr.setPen(b);

  if (clickidx==-2) {
    ptr.drawLine(shgeom.radiusAnchor1, shgeom.radiusAnchor2);
  } else {
    for (auto const &p: geom.transformedNodes) 
      ptr.drawEllipse(p, POINTRADIUS, POINTRADIUS);

    QVector<qreal> pat; pat << 1 << 10;  
    b.setDashPattern(pat);
    ptr.setPen(b);
    ptr.drawPolygon(shgeom.transformedCurve.points);
  }
  
  b.setColor(QColor(255, 0, 0));
  b.setStyle(Qt::SolidLine);
  ptr.setPen(b);
  //  ptr.drawLine(ppp[idx], out);
  ptr.drawEllipse(shgeom.radiusNode, POINTRADIUS, POINTRADIUS);
}

void SO_Layer::paintClone(LayerGeomBase const &geom) {
  QPainter ptr(this);
  int N = geom.transformedRadii.size();
  QPen b(QColor(255,0,0));
  b.setWidth(3);
  ptr.setPen(b);
  for (int n=0; n<N; n++)
    ptr.drawEllipse(geom.transformedNodes[n], POINTRADIUS, POINTRADIUS);

  QVector<qreal> pat; pat << 1 << 10;  

  b.setColor(QColor(0, 200, 0));
  b.setDashPattern(pat);
  ptr.setPen(b);
  for (int n=0; n<N; n++) 
    ptr.drawEllipse(geom.transformedNodes[n+N],
                    geom.transformedRadii[n],
                    geom.transformedRadii[n]);

}

void SO_Layer::mouseReleaseEvent(QMouseEvent *e) {
  clickidx = -1;
  e->accept();
}

void SO_Layer::mouseDoubleClickEvent(QMouseEvent *e) {
  if (e->button()!=Qt::LeftButton) {
    e->ignore();
    return;
  }

  LayerGeomBase geom(this);
  int N = geom.transformedNodes.size();

  if (layer.type()==Layer::Type::Area && N>=4) {
    // consider deleting a point
    for (int k=0; k<N; k++) {
      QPointF p = geom.transformedNodes[k];
      double norm = L2norm(e->pos() - p);
      pDebug() << "compare" << k << norm;
      if (norm < POINTRADIUS*POINTRADIUS) {
        QPolygon pts = layer.points();
        pts.remove(k);
        layer.setPointsAndRadii(pts, layer.radii());
        Layers(vsn, db).setLayer(lay, layer);
        update();
        emit layerMaskChanged(vsn, lay);
        e->accept();
        return;
      }
    }
  }
}

void SO_Layer::mousePressEvent(QMouseEvent *e) {
  if (e->button()!=Qt::LeftButton) {
    e->ignore();
    return;
  }

  LayerGeomBase geom(this);
  int N = geom.transformedNodes.size();
  if (layer.type()==Layer::Type::Clone)
    N /= 2; // convert # of points to # of circles
  pDebug() << "mousepress N="<<N;

  double nearestNorm = 1e9;

  if (layer.type()==Layer::Type::Area) {
    ShapeLayerGeom shgeom(geom);
    double norm = L2norm(e->pos() - shgeom.radiusNode);
    if (norm < nearestNorm) // yes, this is always true, but code may change
      nearestNorm = norm; 
    if (norm < POINTRADIUS*POINTRADIUS) {
      clickidx = -2; // magic
      clickpos = e->pos();
      origpt2 = shgeom.radiusAnchor; // magic
      origpos = shgeom.radiusNode;
      clickscale = geom.radiusFactor;
      layer = Layers(vsn, db).layer(lay); // in case something changed
      e->accept();
      return;
    }
  }

  // This is for all layer types
  for (int k=0; k<N; k++) {
    QPointF p = geom.transformedNodes[k];
    double norm = L2norm(e->pos() - p);
    if (norm < nearestNorm)
      nearestNorm = norm;
    pDebug() << "compare" << k << norm;
    if (norm < POINTRADIUS*POINTRADIUS) {
      clickidx = k;
      clickpos = e->pos();
      origpos = p;
      layer = Layers(vsn, db).layer(lay); // in case something changed
      e->accept();
      return;
    }
  }

  if (layer.type()==Layer::Type::Clone) {
    for (int k=0; k<N; k++) {
      QPointF center = geom.transformedNodes[k+N];
      double r = geom.transformedRadii[k];
      /* How to calculate distance between a point P and the nearest
         point Q on the rim of a circle centered at C with radius R?
         For simplicity, first set C=(0,0) and R=1.
         Circle edge is (cos(phi), sin(phi)) for phi in 0..2pi
         Can do all kinds of fancyness, but easiest is to just calculate
         phi from atan2(P.y, P.x) and R separately.
         In fact, I don't even care about phi for the purpose of knowing
         whether I want to move this circle, only for resizing it.
       */
      QPointF dist = e->pos() - center;
      double d = euclideanLength(dist);
      if (fabs(d-r) < POINTRADIUS) {
        // got it
        if (e->modifiers() & Qt::ShiftModifier) {
          // resize
          clickidx = -3 - k;
          // find point on radius closest to click
          double phi = atan2(dist.y(), dist.x());
          origpt2 = center + QPointF(r*cos(phi), r*sin(phi));
          clickscale = geom.radiusFactor;
        } else {
          // move
          clickidx = k + N;
          origpt2 = geom.transformedNodes[k]; // source
        }
        clickpos = e->pos(); // target center
        origpos = center;
        layer = Layers(vsn, db).layer(lay); // in case something changed
        e->accept();
        return;
      }
    }
  }

  if (layer.type()==Layer::Type::Area
      && nearestNorm >= 4*POINTRADIUS*POINTRADIUS) {
    // consider adding a new control point
    int bestidx = -1;
    ShapeLayerGeom shgeom(geom);
    int M = shgeom.transformedCurve.points.size();
    for (int m=0; m<M; m++) {
      QPointF p = shgeom.transformedCurve.points[m];
      double norm = L2norm(e->pos() - p);
      if (norm < nearestNorm) {
        nearestNorm = norm;
        bestidx = m;
      }
    }
    if (nearestNorm < POINTRADIUS*POINTRADIUS) {
      int after = 0;
      for (int n=1; n<N; n++)
        if (bestidx>shgeom.transformedCurve.origidx[n])
          after = n;
      QPolygon pts = layer.points();
      QTransform const &ixf = base()->transformationToImage();
      QPointF p(ixf.map(shgeom.transformedCurve.points[bestidx]));
      p = Geometry::mapFromAdjusted(p, osize, adj);
      pts.insert(after+1, p.toPoint());
      layer.setPointsAndRadii(pts, layer.radii());
      Layers(vsn, db).setLayer(lay, layer);
      update();
      pDebug() << "inserted new point at" << bestidx << after << pts;
      mousePressEvent(e); // that should do it, right?
      return;
    }
  }
  
  e->ignore();
}

void SO_Layer::mouseMoveEvent(QMouseEvent *e) {
  if (clickidx==-1) {
    e->ignore();
    return;
  }
  
  if (clickidx==-2) {
    QPointF rnode = origpos + e->pos() - clickpos;
    double radius = clickscale * euclideanLength(rnode - origpt2);
    QList<int> rr; rr << radius;
    layer.setPointsAndRadii(layer.points(), rr);
  } else if (clickidx<-2) {
    // this is resizing a clone (or heal?) circle
    QPointF rnode = origpt2 + e->pos() - clickpos;
    double radius = clickscale * euclideanLength(rnode - origpos);
    QList<int> rr = layer.radii();
    pDebug() << "move" << origpos << origpt2 << e->pos() << clickpos << rnode << radius << rr[-3-clickidx] << " at " << -3-clickidx;
    rr[-3-clickidx] = radius + .5;
    layer.setPointsAndRadii(layer.points(), rr);
  } else {
    QTransform const &ixf = base()->transformationToImage();
    QPointF newpt = Geometry::mapFromAdjusted(ixf.map(origpos
                                                      + e->pos() - clickpos),
                                              osize, adj);

    QPolygon poly = layer.points();
    /// QPointF oldpt = poly[clickidx];
    poly[clickidx] = newpt.toPoint();

    if (layer.type()==Layer::Type::Clone
        && clickidx>=layer.points().size()/2) {
      // pull source along
      QPointF newpt =  Geometry::mapFromAdjusted(ixf.map(origpt2
                                                    + e->pos() - clickpos),
                                              osize, adj);
    poly[clickidx - layer.points().size()/2] = newpt.toPoint();
    }

    layer.setPointsAndRadii(poly, layer.radii());
  }

  Layers(vsn, db).setLayer(lay, layer);
  update();
  pDebug() << "SO_Layer: layermaskchanged";
  emit layerMaskChanged(vsn, lay);
  e->accept();
}
