// Slide.h

#ifndef SLIDE_H

#define SLIDE_H

#include <QGraphicsItem>

class Slide: public QGraphicsItem {
public:
  Slide(QGraphicsItem *parent=0);
  void updateImage(QImage const &img);
  void setTileSize(int pix);
  // exif stuff!
  virtual QRectF boundingRect() const override {
    return QRectF(0, 0, tilesize, tilesize);
  }
  virtual void paint(QPainter *painter,
		     const QStyleOptionGraphicsItem *option,
		     QWidget *widget=0) override;
private:
  QImage img;
  QPixmap pm;
  int tilesize;
};

#endif
