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
  virtual void mouseDoubleClickEvent(QMouseEvent *) override;
  virtual void mouseMoveEvent(QMouseEvent *) override;
  virtual void mouseReleaseEvent(QMouseEvent *) override;
signals:
  void layerMaskChanged(quint64 vsn, int lay);
private:
  void paintLinear(class LayerGeomBase const &);
  void paintCurve(class LayerGeomBase const &);
  void paintArea(class LayerGeomBase const &);
  void paintClone(class LayerGeomBase const &);
  void paintInpaint(class LayerGeomBase const &);
  bool perhapsStartDragAreaRadius(QPoint pos, LayerGeomBase const &geom,
                                  double &nearestNorm);
  bool perhapsStartDragPoint(QPoint pos, LayerGeomBase const &geom,
                             int N, double &nearestNorm);
  bool perhapsStartDragCircleEdge(QPoint pos, LayerGeomBase const &geom,
                                  int N, Qt::KeyboardModifiers m,
                                  double &nearestNorm);
  bool perhapsAddControlPoint(QPoint pos, LayerGeomBase const &geom,
                              int N, double &nearestNorm);
  bool perhapsDeleteAreaPoint(QPoint pos, LayerGeomBase const &geom);
  bool perhapsDeleteInpaint(QPoint pos, LayerGeomBase const &geom);
  bool perhapsAddInpaint(QPoint pos, LayerGeomBase const &geom);
  
    
private:
  PhotoDB *db;
  quint64 vsn;
  int lay;
  int clickidx; // magic: -2 means anchorNode
  QPoint clickpos; // widget coord of click
  QPointF origpt2; // widget coords of some other relevant point before move
  QPointF origpos; // widget coords of point before move
  QPoint lastpos; // widget coord of last mouseevent, only when dragsourcealong
  double clickscale; // multiplier for 
  friend class LayerGeomBase;
  Layer layer;
  QSize osize;
  Adjustments adj;
  bool dragsourcealong;
};

#endif
