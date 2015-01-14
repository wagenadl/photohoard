// Slidestrip.h

#ifndef SLIDESTRIP_H

#define SLIDESTRIP_H

#include "Strip.h"

class Slidestrip: public Strip {
  Q_OBJECT;
public:
  Slidestrip(PhotoDB const &db, QGraphicsItem *parent);
  virtual ~Slidestrip();
public:
  virtual QRectF subBoundingRect() const; // children
  virtual void paint(QPainter *painter,
		     const QStyleOptionGraphicsItem *option,
		     QWidget *widget=0) override;
  virtual class Slide *slideByVersion(quint64 vsn);
public slots:
  virtual void setArrangement(Arrangement arr);
  virtual void setTileSize(int pix);
  virtual void setRowWidth(int pix);
  virtual void expand();
  virtual void collapse();
signals:
  void overfilled(QDateTime); // emitted when we have too many pictures
public: // for slide only
  void slidePressed(quint64, Qt::MouseButton, Qt::KeyboardModifiers);
  void slideClicked(quint64, Qt::MouseButton, Qt::KeyboardModifiers);
  void slideDoubleClicked(quint64, Qt::MouseButton, Qt::KeyboardModifiers);
protected slots:
  virtual void relayout();
protected:
  virtual void clearContents();
  virtual void rebuildContents();
  void instantiate();
protected:
  QList<quint64> latentVersions;
  bool hasLatent;
  QMap<quint64, class Slide *> slideMap;
  QList<class Slide *> slideOrder;
  bool mustRelayout;
  bool mustRebuild;
};

#endif
