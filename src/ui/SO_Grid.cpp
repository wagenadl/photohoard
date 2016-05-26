// SO_Grid.cpp

#include "SO_Grid.h"

SO_Grid::SO_Grid(SlideView *sv): SlideOverlay(sv) { }

void SO_Grid::paintEvent(QPaintEvent *) {
  QPainter ptr(this);
  QTransform const &xf = base()->transformationFromImage();
  QSize imgSize = base()->currentImageSize();
  double w = imgSize.width();
  double h = imgSize.height();
  ptr.setPen(QPen(QColor(128, 128, 128, 128)));
  ptr.drawLine(xf.map(QLineF(QPointF(0, h/3), QPointF(w, h/3))));
  ptr.drawLine(xf.map(QLineF(QPointF(0, 2*h/3), QPointF(w, 2*h/3))));
  ptr.drawLine(xf.map(QLineF(QPointF(w/3, 0), QPointF(w/3, h))));
  ptr.drawLine(xf.map(QLineF(QPointF(2*w/3, 0), QPointF(2*w/3, h))));
}

