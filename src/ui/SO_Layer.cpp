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
    int idx;
    transformedCurve = Spline::catmullRom(base.transformedNodes, 2, &idx);
    radiusAnchor = transformedCurve[idx];

    int N = transformedCurve.size();
    int n1 = idx + 1;
    if (n1>=N)
      n1 -= N;
    int n2 = idx - 1;
    if (n2<0)
      n2 += N;
    QPointF dir(transformedCurve[n1] - transformedCurve[n2]);
    dir /= euclideanLength(dir);
    radiusNode = QPointF(transformedCurve[idx]
                         + QPointF(-dir.y(), dir.x())
                           * base.transformedRadii[0]);
  }
public:
  QPolygonF transformedCurve;
  QPointF radiusAnchor; // on transformedCurve
  QPointF radiusNode; // at a distance
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
  default:
    pDebug() << "Paint Layer type" << int(layer.type()) << "NYI";
    break;
  }
}

constexpr static int POINTRADIUS = 10;

void SO_Layer::paintLinear(LayerGeomBase const &geom) {
  QPainter ptr(this);
  bool first = true;
  QPen b(QColor(255,0,0));
  b.setWidth(3);
  ptr.setPen(b);
  for (auto const &p: geom.transformedNodes) {
    ptr.drawEllipse(p, POINTRADIUS, POINTRADIUS);
    if (first) {
      b.setColor(QColor(0,200,0));
      ptr.setPen(b);
      first = false;
    }
  }
}

void SO_Layer::paintCircular(LayerGeomBase const &geom) {
}

void SO_Layer::paintCurve(LayerGeomBase const &geom) {
}

void SO_Layer::paintArea(LayerGeomBase const &geom) {
  ShapeLayerGeom shgeom(geom);
  
  QPainter ptr(this);
  QPen b(QColor(0,200,0));

  b.setWidth(3);
  ptr.setPen(b);
  for (auto const &p: geom.transformedNodes) 
    ptr.drawEllipse(p, POINTRADIUS, POINTRADIUS);

  //  b.setWidth(2);
  QVector<qreal> pat; pat << 1 << 10;  
  b.setDashPattern(pat);
  ptr.setPen(b);
  ptr.drawPolygon(shgeom.transformedCurve);

  b.setColor(QColor(255, 0, 0));
  b.setStyle(Qt::SolidLine);
  ptr.setPen(b);
  //  ptr.drawLine(ppp[idx], out);
  ptr.drawEllipse(shgeom.radiusNode, POINTRADIUS, POINTRADIUS);
}

void SO_Layer::paintHeal(LayerGeomBase const &) {
}

void SO_Layer::mouseReleaseEvent(QMouseEvent *e) {
  clickidx = -1;
  e->accept();
}

void SO_Layer::mousePressEvent(QMouseEvent *e) {
  if (e->button()!=Qt::LeftButton) {
    e->ignore();
    return;
  }

  LayerGeomBase geom(this);
  int N = geom.transformedNodes.size();

  if (layer.type()==Layer::Type::Area) {
    ShapeLayerGeom shgeom(geom);
    if (L2norm(e->pos() - shgeom.radiusNode) < POINTRADIUS*POINTRADIUS) {
      clickidx = -2; // magic
      clickpos = e->pos();
      origpt = shgeom.radiusAnchor; // magic
      origpos = shgeom.radiusNode;
      clickscale = geom.radiusFactor;
      layer = Layers(vsn, db).layer(lay); // in case something changed
      e->accept();
      return;
    }
  }
  
  for (int k=0; k<N; k++) {
    QPointF p = geom.transformedNodes[k];
    if (L2norm(e->pos() - p) < POINTRADIUS*POINTRADIUS) {
      clickidx = k;
      clickpos = e->pos();
      origpt = layer.points()[k];
      origpos = p;
      layer = Layers(vsn, db).layer(lay); // in case something changed
      e->accept();
      return;
    }
  }
  
  e->ignore();
}

void SO_Layer::mouseMoveEvent(QMouseEvent *e) {
  switch (clickidx) {
  case -1:
    e->ignore();
    return;
  case -2: {
    QPointF rnode = origpos + e->pos() - clickpos;
    double radius = clickscale * euclideanLength(rnode - origpt);
    QList<int> rr; rr << radius;
    layer.setPointsAndRadii(layer.points(), rr);
  } break;
  default: {
    QTransform const &ixf = base()->transformationToImage();
    QPointF p0 = ixf.map(clickpos);
    QPointF p1 = ixf.map(e->pos());
    QPointF newpt = origpt + p1 - p0; // but of course we should properly xform:
    // AdjusterGeometry::revmap(ixf.map(e->pos() + origpos - clickpos))
    
    QPolygon poly = layer.points();
    /// QPointF oldpt = poly[clickidx];
    poly[clickidx] = newpt.toPoint();
    layer.setPointsAndRadii(poly, layer.radii());
  } break;
  }

  Layers(vsn, db).setLayer(lay, layer);
  update();
  pDebug() << "SO_Layer: layermaskchanged";
  emit layerMaskChanged(vsn, lay);
  e->accept();
}
