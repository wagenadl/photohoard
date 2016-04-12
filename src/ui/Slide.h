// Slide.h

#ifndef SLIDE_H

#define SLIDE_H

#include <QGraphicsItem>
#include <QPointer>

#include "Slidestrip.h"

class Slide: public QGraphicsItem {
public:
  Slide(quint64 id, class Slidestrip *parent=0);
  virtual ~Slide();
  quint64 version() const { return id; }
  void makeReady();
  void reload();
  void updateImage(Image16 const &img, bool chgd);
  void quickRotate(int dphi);
  void setTileSize(int pix);
  virtual QRectF boundingRect() const override {
    return QRectF(0, 0, tilesize, tilesize);
  }
  virtual void paint(QPainter *painter,
		     const QStyleOptionGraphicsItem *option,
		     QWidget *widget=0) override;
  virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *) override;
  virtual void mousePressEvent(QGraphicsSceneMouseEvent *) override;
  virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *) override;
  Slidestrip *parentStrip() const;
private:
  static QColor colorLabelColor(int);
private:
  QPointer<Slidestrip> parent;
  quint64 id;
  Image16 img;
  QPixmap pm;
  int tilesize;
};

#endif
