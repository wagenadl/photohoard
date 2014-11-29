// Strip.h

#ifndef STRIP_H

#define STRIP_H

#include <QGraphicsObject>
#include "PhotoDB.h"
#include <QDateTime>

class Strip: public QGraphicsObject {
  Q_OBJECT;
public:
  enum class Arrangement {
    Horizontal,
    Vertical,
    Grid,
  };
  enum class TimeScale {
    Eternity,
    Decade,
    Year,
    Month,
    Day,
    Hour,
    DecaMinute,
    None,
  }; 
public:
  Strip(PhotoDB const &db, QGraphicsItem *parent);
  virtual ~Strip();
  QDateTime startDateTime() const;
  QDateTime endDateTime() const;
  TimeScale timeScale() const;
  Arrangement arrangement() const { return arr; }
  bool isExpanded() const { return expanded; }
  int subRowWidth(int pix) const;
  bool hasTopLabel() const;
  virtual class Slide *slideByVersion(quint64 vsn) const;
public slots:
  void rescan();
  void updateImage(quint64, QSize, QImage);
  void updateHeader(QImage);
  void setTimeRange(QDateTime t0, TimeScale scl);
  virtual void setArrangement(Arrangement arr);
  virtual void setTileSize(int pix);
  virtual void setRowWidth(int pix);
  virtual void expand();
  virtual void collapse();
  virtual void expandAll();
  virtual void collapseAll();
signals:
  void needImage(quint64, QSize);
  void resized();
  void pressed(quint64);
  void clicked(quint64);
  void doubleClicked(quint64);
public:
  virtual QRectF boundingRect() const override; // just us, i.e., our label
  virtual QRectF labelBoundingRect() const; // just us
  virtual QRectF netBoundingRect() const; // us and children
  virtual QRectF subBoundingRect() const; // children
  virtual void paint(QPainter *painter,
		     const QStyleOptionGraphicsItem *option,
		     QWidget *widget=0) override;
public:
  static QDateTime startFor(QDateTime d, TimeScale scl);
  static QDateTime endFor(QDateTime start, TimeScale scl);
  static QString labelFor(QDateTime t0, TimeScale scl);
  static int labelHeight(int tilesize);
protected:
  virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *) override;
  virtual void mousePressEvent(QGraphicsSceneMouseEvent *) override;
  virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *) override;
protected slots:
  virtual void relayout();
protected:
  virtual void clearContents();
  virtual void rebuildContents();
  void setHeaderID(quint64);
protected:
  int countInRange(QDateTime t0, QDateTime t1) const;
  QList<quint64> versionsInRange(QDateTime t0, QDateTime t1) const;
  QDateTime firstDateInRange(QDateTime t0, QDateTime t1) const;
  QDateTime lastDateInRange(QDateTime t0, QDateTime t1) const;
protected:
  bool shouldDebug() const;
  int subHeight() const;
protected:
  PhotoDB db;
  QDateTime d0;
  TimeScale scl;
  Arrangement arr;
  int tilesize;
  int rowwidth;
  bool expanded;
  mutable int subheight;
  quint64 headerid;
  QImage headerimg;
  QPixmap headerpm;
};

#endif
