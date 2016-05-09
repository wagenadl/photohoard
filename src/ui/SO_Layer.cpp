// SO_Layer.cpp

#include "SO_Layer.h"
#include <QPainter>
#include "AdjusterGeometry.h"
#include <QDebug>
#include <QMouseEvent>
#include "PhotoDB.h"

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

void SO_Layer::render(QPainter *ptr, QRect const &) {
  QTransform const &xf = base()->transformationFromImage();
  QPolygonF poly = layer.points();
  qDebug() << "SO_Layer::render" << poly;
  for (auto &p: poly)
    p = xf.map(AdjusterGeometry::map(p, osize, adj));
  qDebug() << "  ->" << poly;

  bool first = true;
  QPen b(QColor(255,0,0));
  b.setWidth(3);
  ptr->setPen(b);
  for (auto const &p: poly) {
    ptr->drawEllipse(p, 10.0, 10.0);
    if (first) {
      b.setColor(QColor(0,200,0));
      ptr->setPen(b);
      first = false;
    }
  }
}

bool SO_Layer::handleRelease(QMouseEvent *) {
  if (clickidx>=0) {
    clickidx = -1;
    return true;
  } else {
    return false;
  }
}

static inline double euclideanLength2(QPointF p) {
  double x = p.x();
  double y = p.y();
  return x*x + y*y;
}

bool SO_Layer::handlePress(QMouseEvent *e) {
  if (e->button()!=Qt::LeftButton)
    return false;

  QTransform const &xf = base()->transformationFromImage();
  QPolygonF poly0 = layer.points();
  QPolygonF poly = poly0;
  qDebug() << "SO_Layer::render" << poly;
  for (auto &p: poly)
    p = xf.map(AdjusterGeometry::map(p, osize, adj));

  for (int idx=0; idx<poly.size(); idx++) {
    if (euclideanLength2(e->pos() - poly[idx]) < 100) {
      clickidx = idx;
      clickpos = e->pos();
      origpt = poly0[idx];
      origpos = poly[idx];
      layer = Layers(vsn, db).layer(lay); // in case something changed
      return true;
    }
  }

  return false;
}

bool SO_Layer::handleMove(QMouseEvent *e) {
  if (clickidx<0)
    return false;

  QTransform const &ixf = base()->transformationToImage();
  QPointF p0 = ixf.map(clickpos);
  QPointF p1 = ixf.map(e->pos());
  QPointF newpt = origpt + p1 - p0; // but of course we should properly xform:
  // AdjusterGeometry::revmap(ixf.map(e->pos() + origpos - clickpos))

  QPolygon poly = layer.points();
  poly[clickidx] = newpt.toPoint();
  layer.setPoints(poly);
  Layers(vsn, db).setLayer(lay, layer);

  base()->update();
  emit layerMaskChanged(vsn, lay);

  return true;
}
