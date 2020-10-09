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
  QPolygonF poly = layer.points();
  if (poly.isEmpty())
    return;

  QTransform const &xf = base()->transformationFromImage();
  
  QPointF p0;
  for (auto &p: poly)
    p0 += p;
  p0 /= poly.size();

  for (auto &p: poly)
    p = xf.map(Geometry::mapToAdjusted(p, osize, adj));

  QList<double> radii;
  for (auto a: layer.radii()) {
    QPointF p1 = p0 + QPointF(a, 0);
    QPointF p0a = xf.map(Geometry::mapToAdjusted(p0, osize, adj));
    QPointF p1a = xf.map(Geometry::mapToAdjusted(p1, osize, adj));
    pDebug() << "radii" << a << p1 << p0a << p1a;
    radii << euclideanLength(p1a-p0a);
  }
  switch (layer.type()) {
  case Layer::Type::LinearGradient:
    paintLinear(poly, radii);
    break;
  case Layer::Type::Area:
    paintArea(poly, radii);
    break;
  default:
    pDebug() << "Paint Layer type" << int(layer.type()) << "NYI";
    break;
  }
}

void SO_Layer::paintLinear(QPolygonF const &poly,
                           QList<double> const &) {
  QPainter ptr(this);
  bool first = true;
  QPen b(QColor(255,0,0));
  b.setWidth(3);
  ptr.setPen(b);
  for (auto const &p: poly) {
    ptr.drawEllipse(p, 10.0, 10.0);
    if (first) {
      b.setColor(QColor(0,200,0));
      ptr.setPen(b);
      first = false;
    }
  }
}

void SO_Layer::paintCircular(QPolygonF const &poly,
                             QList<double> const &radii) {
}

void SO_Layer::paintCurve(QPolygonF const &poly,
                          QList<double> const &radii) {
}

void SO_Layer::paintArea(QPolygonF const &poly,
                         QList<double> const &radii) {
  QPainter ptr(this);
  QPen b(QColor(0,200,0));

  b.setWidth(3);
  ptr.setPen(b);
  for (auto const &p: poly) 
    ptr.drawEllipse(p, 10.0, 10.0);

  //  b.setWidth(2);
  QVector<qreal> pat; pat << 1 << 10;  
  b.setDashPattern(pat);
  ptr.setPen(b);
  int idx;
  QPolygonF ppp = Spline::catmullRom(poly, 2, &idx);
  ptr.drawPolygon(ppp);

  b.setColor(QColor(255, 0, 0));
  b.setStyle(Qt::SolidLine);
  ptr.setPen(b);
  int N = ppp.size();
  int n1 = idx + 1;
  if (n1>=N)
    n1 -= N;
  int n2 = idx - 1;
  if (n2<0)
    n2 += N;
  QPointF dir(ppp[n1] - ppp[n2]);
  dir /= euclideanLength(dir);
  QPointF out(ppp[idx] + QPointF(-dir.y(), dir.x())*radii[0]);
  //  ptr.drawLine(ppp[idx], out);
  ptr.drawEllipse(out, 10, 10);
}

void SO_Layer::paintHeal(QPolygonF const &poly,
                         QList<double> const &radii) {
}

void SO_Layer::mouseReleaseEvent(QMouseEvent *e) {
  if (clickidx>=0) {
    clickidx = -1;
    e->accept();
  } else {
    e->ignore();
  }
}

void SO_Layer::mousePressEvent(QMouseEvent *e) {
  if (e->button()!=Qt::LeftButton) {
    e->ignore();
    return;
  }

  QTransform const &xf = base()->transformationFromImage();
  QPolygonF poly0 = layer.points();
  QPolygonF poly = poly0;
  //pDebug() << "SO_Layer::render" << poly;
  for (auto &p: poly)
    p = xf.map(Geometry::mapToAdjusted(p, osize, adj));

  for (int idx=0; idx<poly.size(); idx++) {
    if (L2norm(e->pos() - poly[idx]) < 100) {
      clickidx = idx;
      clickpos = e->pos();
      origpt = poly0[idx];
      origpos = poly[idx];
      layer = Layers(vsn, db).layer(lay); // in case something changed
      e->accept();
      return;
    }
  }
  
  e->ignore();
}

void SO_Layer::mouseMoveEvent(QMouseEvent *e) {
  if (clickidx<0) {
    e->ignore();
    return;
  }

  QTransform const &ixf = base()->transformationToImage();
  QPointF p0 = ixf.map(clickpos);
  QPointF p1 = ixf.map(e->pos());
  QPointF newpt = origpt + p1 - p0; // but of course we should properly xform:
  // AdjusterGeometry::revmap(ixf.map(e->pos() + origpos - clickpos))

  QPolygon poly = layer.points();
  /// QPointF oldpt = poly[clickidx];
  poly[clickidx] = newpt.toPoint();
  layer.setPointsAndRadii(poly, layer.radii());
  Layers(vsn, db).setLayer(lay, layer);

  update();
  pDebug() << "SO_Layer: layermaskchanged";
  emit layerMaskChanged(vsn, lay);

  e->accept();
}
