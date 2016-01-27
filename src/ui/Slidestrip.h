// Slidestrip.h

#ifndef SLIDESTRIP_H

#define SLIDESTRIP_H

#include "Strip.h"
#include <QHash>

class Slidestrip: public Strip {
  Q_OBJECT;
public:
  Slidestrip(PhotoDB *db, QGraphicsItem *parent);
  virtual ~Slidestrip();
public:
  virtual QRectF subBoundingRect() const; // children
  virtual void paint(QPainter *painter,
		     const QStyleOptionGraphicsItem *option,
		     QWidget *widget=0) override;
  virtual class Slide *slideByVersion(quint64 vsn);
  virtual quint64 versionAt(quint64 vsn, QPoint dxy);
  QPoint gridPosition(quint64 vsn); // returns (-1,-1) if not found
  quint64 versionAt(QPoint cr);
  virtual quint64 firstExpandedVersion();
  virtual quint64 lastExpandedVersion();
  virtual Strip *firstExpandedStrip();
  virtual Strip *lastExpandedStrip();
  static int threshold();
  int count() const;
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
  QMap<quint64, QPoint> placement; // (x,y) are integer grid positions, not 
  QHash<QPoint, quint64> revplace; // scene or item coordinates
  int maxcplace;
  bool mustRelayout;
  bool mustRebuild;
  QRectF oldbb;
};

#endif
