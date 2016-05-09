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
  virtual void render(class QPainter *ptr, class QRect const &rect);
  virtual bool handlePress(QMouseEvent *) override;
  virtual bool handleMove(QMouseEvent *) override;
  virtual bool handleRelease(QMouseEvent *) override;
signals:
  void layerMaskChanged(quint64 vsn, int lay);
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
