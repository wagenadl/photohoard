// Slide.cpp

#include "Slide.h"
#include <QPainter>

Slide::Slide(QGraphicsItem *parent): QGraphicsItem(parent) {
  tilesize = 128;
}

void Slide::updateImage(QImage const &img1) {
  img = img1;
  pm = QPixmap();
  update();
}

void Slide::setTileSize(int pix) {
  tilesize = pix;
  update();
}

void Slide::paint(QPainter *painter,
		  const QStyleOptionGraphicsItem *,
		  QWidget *) {
  QRectF r = boundingRect();
  painter->setPen(QPen(Qt::NoPen));
  painter->setBrush(QBrush(QColor(0, 0, 0)));
  painter->drawRoundedRect(r.adjusted(2, 2, 0, 0), 4, 4);
  painter->setBrush(QBrush(QColor(255, 255, 255)));
  painter->drawRoundedRect(r.adjusted(0, 0, -2, -2), 4, 4);
  painter->setBrush(QBrush(QColor(127, 127, 127)));
  painter->drawRoundedRect(r.adjusted(1, 1, -1, -1), 4, 4);

  int ims = tilesize - 4;
  if (!(pm.width()==ims || pm.height()==ims)) 
    pm = QPixmap::fromImage(img.scaled(QSize(ims, ims),
				       Qt::KeepAspectRatio));

  painter->drawPixmap(tilesize/2 - pm.width()/2,
		      tilesize/2 - pm.height()/2,
		      pm);
}
