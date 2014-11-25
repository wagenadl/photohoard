// Slide.h

#ifndef SLIDE_H

#define SLIDE_H

#include <QGraphicsItem>
#include <QPointer>

#include "Filmstrip.h"

class Slide: public QGraphicsItem {
public:
  Slide(quint64 id, class Filmstrip *parent=0);
  void makeReady();
  void updateImage(QImage const &img);
  void setTileSize(int pix);
  // exif stuff!
  virtual QRectF boundingRect() const override {
    return QRectF(0, 0, tilesize, tilesize);
  }
  virtual void paint(QPainter *painter,
		     const QStyleOptionGraphicsItem *option,
		     QWidget *widget=0) override;
  virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *) override;
  virtual void mousePressEvent(QGraphicsSceneMouseEvent *) override;
  virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *) override;
private:
  QPointer<Filmstrip> parent;
  quint64 id;
  QImage img;
  QPixmap pm;
  int tilesize;
};

#endif
