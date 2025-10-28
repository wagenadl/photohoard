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
    QPolygon pts0 = so->layer.points();
    QPolygonF poly(pts0);
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
    transformedCurve = Spline(base.transformedNodes, 2);
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
  case Layer::Type::Inpaint:
    paintInpaint(geom);
    break;
  default:
    qWarning() << "Paint Layer type" << int(layer.type()) << "NYI";
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

void SO_Layer::paintCurve(LayerGeomBase const &/*geom*/) {
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
  ptr.drawEllipse(geom.transformedNodes[N], POINTRADIUS, POINTRADIUS);

  QVector<qreal> pat; pat << 1 << 10;  

  b.setColor(QColor(100, 200, 0));
  b.setDashPattern(pat);
  ptr.setPen(b);
  for (int n=0; n<N; n++) {
    ptr.drawEllipse(geom.transformedNodes[n],
                    geom.transformedRadii[n],
                    geom.transformedRadii[n]);
    b.setColor(QColor(0, 200, 0));
  }

}

void SO_Layer::paintInpaint(LayerGeomBase const &geom) {
  QPainter ptr(this);
  int N = geom.transformedRadii.size();

  QVector<qreal> pat; pat << 1 << 10;  

  QPen b(QColor(0, 200, 0));
  b.setWidth(3);
  b.setDashPattern(pat);
  ptr.setPen(b);
  for (int n=0; n<N; n++) 
    ptr.drawEllipse(geom.transformedNodes[n],
                    geom.transformedRadii[n],
                    geom.transformedRadii[n]);

}

void SO_Layer::mouseReleaseEvent(QMouseEvent *e) {
  clickidx = -1;
  e->accept();
}

bool SO_Layer::perhapsDeleteAreaPoint(QPoint pos, LayerGeomBase const &geom) {
  int N = geom.transformedNodes.size();
  if (N < 4)
    return false;
  for (int k=0; k<N; k++) {
    QPointF p = geom.transformedNodes[k];
    double norm = L2norm(pos - p);
    double r = geom.transformedRadii[k];
    if (norm < r*r* + POINTRADIUS*POINTRADIUS) {
      QPolygon pts = layer.points();
      pts.remove(k);
      QList<int> rdi = layer.radii();
      layer.setPointsAndRadii(pts, rdi);
      Layers(vsn, db).setLayer(lay, layer);
      update();
      emit layerMaskChanged(vsn, lay);
      return true;
    }
  }
  return false;
}

bool SO_Layer::perhapsDeleteInpaint(QPoint pos, LayerGeomBase const &geom) {
  int N = geom.transformedRadii.size();
  if (N>=2) {
    for (int k=0; k<N; k++) {
      QPointF p = geom.transformedNodes[k];
      double norm = L2norm(pos - p);
      double r = geom.transformedRadii[k];
      if (norm < r*r + POINTRADIUS*POINTRADIUS) { // in or very near circle
        QPolygon pts = layer.points();
        pts.remove(k);
        QList<int> rdi = layer.radii();
        rdi.removeAt(k);
        layer.setPointsAndRadii(pts, rdi);
        Layers(vsn, db).setLayer(lay, layer);
        update();
        emit layerMaskChanged(vsn, lay);
        return true;
      }
    }
  }
  return false;
}

bool SO_Layer::perhapsAddInpaint(QPoint pos, LayerGeomBase const &/*geom*/) {
  QPolygon pts = layer.points();
  QTransform const &ixf = base()->transformationToImage();
  QPointF p(ixf.map(pos));
  pts.insert(layer.radii().size(),
             Geometry::mapFromAdjusted(p, osize, adj).toPoint());
  QList<int> rdi = layer.radii();
  rdi << rdi[rdi.size()-1];
  layer.setPointsAndRadii(pts, rdi);
  Layers(vsn, db).setLayer(lay, layer);
  update();
  emit layerMaskChanged(vsn, lay);
  return true;
}


void SO_Layer::mouseDoubleClickEvent(QMouseEvent *e) {
  if (e->button()!=Qt::LeftButton) {
    e->ignore();
    return;
  }

  LayerGeomBase geom(this);

  if (layer.type()==Layer::Type::Area) {
    if (perhapsDeleteAreaPoint(e->pos(), geom)) {
      e->accept();
      return;
    }
  } else if (layer.type()==Layer::Type::Clone
             || layer.type()==Layer::Type::Inpaint) {
    if (perhapsDeleteInpaint(e->pos(), geom)
        || perhapsAddInpaint(e->pos(), geom)) {
      e->accept();
      return;
    }
  }
}

bool SO_Layer::perhapsStartDragAreaRadius(QPoint pos, LayerGeomBase const &geom,
                                          double &nearestNorm) {
  ShapeLayerGeom shgeom(geom);
  double norm = L2norm(pos - shgeom.radiusNode);
  if (norm < nearestNorm) 
    nearestNorm = norm; 
  if (norm < POINTRADIUS*POINTRADIUS) {
    clickidx = -2; // magic
    clickpos = pos;
    origpt2 = shgeom.radiusAnchor; // magic
    origpos = shgeom.radiusNode;
    clickscale = geom.radiusFactor;
    layer = Layers(vsn, db).layer(lay); // in case something changed
    return true;
  }
  return false;
}

bool SO_Layer::perhapsStartDragPoint(QPoint pos, LayerGeomBase const &geom,
                                     int N, double &nearestNorm) {
  for (int k=0; k<N; k++) {
    QPointF p = geom.transformedNodes[k];
    double norm = L2norm(pos - p);
    if (norm < nearestNorm)
      nearestNorm = norm;
    if (norm < POINTRADIUS*POINTRADIUS) {
      clickidx = k;
      clickpos = pos;
      origpos = p;
      layer = Layers(vsn, db).layer(lay); // in case something changed
      return true;
    }
  }
  return false;
}  

bool SO_Layer::perhapsStartDragCircleEdge(QPoint pos, LayerGeomBase const &geom,
                                          int N, Qt::KeyboardModifiers m,
                                          double &nearestNorm) {
  for (int k=0; k<N; k++) {
    QPointF center = geom.transformedNodes[k];
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
    QPointF dist = pos - center;
    double d = euclideanLength(dist);
    if (fabs(d-r) < POINTRADIUS) {
      // got it
      double nrm = (d-r)*(d-r);
      if (nrm < nearestNorm)
        nearestNorm = nrm;
      if (m & Qt::ShiftModifier) {
        // resize
        clickidx = -3 - k;
        // find point on radius closest to click
        double phi = atan2(dist.y(), dist.x());
        origpt2 = center + QPointF(r*cos(phi), r*sin(phi));
        clickscale = geom.radiusFactor;
      } else {
        // move
        clickidx = k;
        if (layer.type()==Layer::Type::Clone && k==0) {
          dragsourcealong = true;
          origpt2 = geom.transformedNodes[N];
        }
      }
      lastpos = clickpos = pos; // target center
      origpos = center;
      layer = Layers(vsn, db).layer(lay); // in case something changed
      return true;
    }
  }
  return false;
}   

bool SO_Layer::perhapsAddControlPoint(QPoint pos, LayerGeomBase const &geom,
                                      int N, double &nearestNorm) {
  if (nearestNorm < 4*POINTRADIUS*POINTRADIUS)
    return false;
  int bestidx = -1;
  ShapeLayerGeom shgeom(geom);
  int M = shgeom.transformedCurve.points.size();
  for (int m=0; m<M; m++) {
    QPointF p = shgeom.transformedCurve.points[m];
    double norm = L2norm(pos - p);
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
    return true;
  }
  return false;
}
 
void SO_Layer::mousePressEvent(QMouseEvent *e) {
  if (e->button()!=Qt::LeftButton) {
    e->ignore();
    return;
  }
  dragsourcealong = false;
  LayerGeomBase geom(this);
  int N = (layer.type()==Layer::Type::Clone)
    ? geom.transformedRadii.size()
    : geom.transformedNodes.size();

  double nearestNorm = 1e9;

  if (layer.type()==Layer::Type::Area) {
    if (perhapsStartDragAreaRadius(e->pos(), geom, nearestNorm)) {
      e->accept();
      return;
    }
  }

  if (perhapsStartDragPoint(e->pos(), geom, geom.transformedNodes.size(),
                            nearestNorm)) {
    e->accept();
    return;
  }

  if (layer.type()==Layer::Type::Clone || layer.type()==Layer::Type::Inpaint) {
    if (perhapsStartDragCircleEdge(e->pos(), geom, N, e->modifiers(),
                                   nearestNorm)) {
      e->accept();
      return;
    }
  }

  if (layer.type()==Layer::Type::Area) {
    if (perhapsAddControlPoint(e->pos(), geom, N, nearestNorm)) {
      LayerGeomBase geom(this);
      int N = geom.transformedNodes.size();
      perhapsStartDragPoint(e->pos(), geom, N, nearestNorm);      
      e->accept();
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
    // dragging radius of area
    QPointF rnode = origpos + e->pos() - clickpos;
    double radius = clickscale * euclideanLength(rnode - origpt2);
    QList<int> rr; rr << radius;
    layer.setPointsAndRadii(layer.points(), rr);
  } else if (clickidx<-2) {
    // resizing a clone or heal circle
    QPointF rnode = origpt2 + e->pos() - clickpos;
    double radius = clickscale * euclideanLength(rnode - origpos);
    QList<int> rr = layer.radii();
    rr[-3-clickidx] = radius + .5;
    layer.setPointsAndRadii(layer.points(), rr);
  } else {
    // dragging a point
    QTransform const &ixf = base()->transformationToImage();
    QPointF newpt = Geometry::mapFromAdjusted(ixf.map(origpos
                                                      + e->pos() - clickpos),
                                              osize, adj);
    QPolygon poly = layer.points();
    poly[clickidx] = newpt.toPoint();
    if (dragsourcealong && !(e->modifiers() & Qt::ControlModifier)) {
      QPointF npt2 = origpt2 + e->pos() - lastpos;
      QPointF newpt =  Geometry::mapFromAdjusted(ixf.map(npt2),
                                                 osize, adj);
      poly[poly.size()-1] = newpt.toPoint();
      origpt2 = npt2;
    }
    lastpos = e->pos();
    layer.setPointsAndRadii(poly, layer.radii());
  }
  Layers(vsn, db).setLayer(lay, layer);
  emit layerMaskChanged(vsn, lay);
  update();
  e->accept();
}
