// SO_Layer.h

#ifndef SO_LAYER_H

#define SO_LAYER_H

#include "SlideOverlay.h"
#include "Layers.h"
#include "Adjustments.h"

class SO_Layer: public SlideOverlay {
  Q_OBJECT;
public:
  SO_Layer(class PhotoDB *db, class SlideView *parent=0);
  void setLayer(quint64 vsnid, int layer);
  void updateTransform();
  virtual void paintEvent(QPaintEvent *) override;
  virtual void mousePressEvent(QMouseEvent *) override;
  virtual void mouseMoveEvent(QMouseEvent *) override;
  virtual void mouseReleaseEvent(QMouseEvent *) override;
signals:
  void layerMaskChanged(quint64 vsn, int lay);
private:
  void paintLinear(QPolygonF const &poly, QList<double> const &radii);
  void paintCircular(QPolygonF const &poly, QList<double> const &radii);
  void paintCurve(QPolygonF const &poly, QList<double> const &radii);
  void paintArea(QPolygonF const &poly, QList<double> const &radii);
  void paintHeal(QPolygonF const &poly, QList<double> const &radii);
private:
  PhotoDB *db;
  quint64 vsn;
  int lay;
  int clickidx;
  QPoint clickpos; // widget coord of click
  QPointF origpt; // image coords of point before move
  QPointF origpos; // widget coords of point before move
  Layer layer;
  QSize osize;
  Adjustments adj;
};

#endif
