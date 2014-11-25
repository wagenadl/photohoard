// Slide.cpp

#include "Slide.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QStyleOptionGraphicsItem>
#include <QDebug>

Slide::Slide(quint64 id, Filmstrip *parent):
  QGraphicsItem(parent), parent(parent), id(id) {
  tilesize = 128;
  setPos(1e6, 1e6);
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
		  const QStyleOptionGraphicsItem *o,
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
  if (!(pm.width()==ims || pm.height()==ims)) {
    if (img.isNull()) {
      painter->setPen(QPen(QColor(255, 255, 255)));
      painter->setBrush(QBrush(QColor(0, 0, 0)));
      if (pm.isNull()) {
	painter->drawText(r, Qt::AlignCenter, QString("%1").arg(id));
      } else {
	double hrat = double(ims) / pm.width();
	double vrat = double(ims) / pm.height();
	double rat = hrat<vrat ? hrat: vrat;
	QSize tgt(rat*pm.width(), rat*pm.height());
	QRect dst(QPoint(tilesize/2-tgt.width()/2,
			   tilesize/2-tgt.height()/2), tgt);
	painter->drawPixmap(dst, pm);
      }
      // qDebug() << id << scenePos() << r << o->exposedRect;
      parent->requestImage(id);
      return;
    } 
    pm = QPixmap::fromImage(img.scaled(QSize(ims, ims),
				       Qt::KeepAspectRatio));
    img = QImage(); // no need to keep it ad inf
  }
  painter->drawPixmap(tilesize/2 - pm.width()/2,
		      tilesize/2 - pm.height()/2,
		      pm);
}

void Slide::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *) {
  if (parent)
    parent->slideDoubleClicked(id);
}

void Slide::mousePressEvent(QGraphicsSceneMouseEvent *e) {
    if (parent)
      parent->slidePressed(id);
  e->accept();
}

void Slide::mouseReleaseEvent(QGraphicsSceneMouseEvent *e) {
  if ((e->screenPos() - e->buttonDownScreenPos(e->button()))
      .manhattanLength() < 5)
    if (parent)
      parent->slideClicked(id);
}

