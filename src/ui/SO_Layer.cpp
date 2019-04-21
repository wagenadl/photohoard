// SO_Layer.cpp

#include "SO_Layer.h"
#include <QPainter>
#include "AdjusterGeometry.h"
#include <QDebug>
#include <QMouseEvent>
#include "PhotoDB.h"
#include "Geometry.h"

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
  QPainter ptr(this);
  QTransform const &xf = base()->transformationFromImage();
  QPolygonF poly = layer.points();
  // pDebug() << "SO_Layer::render" << poly;
  for (auto &p: poly)
    p = xf.map(Geometry::mapToAdjusted(p, osize, adj));
  //pDebug() << "  ->" << poly;

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

void SO_Layer::mouseReleaseEvent(QMouseEvent *e) {
  if (clickidx>=0) {
    clickidx = -1;
    e->accept();
  } else {
    e->ignore();
  }
}

static inline double euclideanLength2(QPointF p) {
  double x = p.x();
  double y = p.y();
  return x*x + y*y;
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
    if (euclideanLength2(e->pos() - poly[idx]) < 100) {
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
  layer.setPoints(poly);
  Layers(vsn, db).setLayer(lay, layer);

  update();
  emit layerMaskChanged(vsn, lay);

  e->accept();
}
