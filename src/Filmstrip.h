// Filmstrip.h

#ifndef FILMSTRIP_H

#define FILMSTRIP_H

#include <QGraphicsObject>
#include "PhotoDB.h"
#include "Slide.h"
#include <QDateTime>

class Filmstrip: public QGraphicsObject {
  Q_OBJECT;
public:
  enum class Arrangement {
    Horizontal,
    Vertical,
    Grid,
  };
  enum class TimeScale {
    None,
    Decade,
    Year,
    Month,
    Day,
    Hour,
    DecaMinute,
  };
public:
  Filmstrip(PhotoDB const &db, QGraphicsItem *parent);
  virtual ~Filmstrip();
  QDateTime startDateTime() const;
  QDateTime endDateTime() const;
  TimeScale timeScale() const;
  virtual QRectF boundingRect() const override; // just us, i.e., our header
  virtual QRectF netBoundingRect() const; // us and expanded children
  virtual void paint(QPainter *painter,
		     const QStyleOptionGraphicsItem *option,
		     QWidget *widget=0) override;
  Filmstrip *descendentByDate(QDateTime d);
  /* Returns the strip that would directly host a picture with given date.
     That might be us or a (grand)child. It can also be NULL if the date
     is outside our range. */
  Slide *slideByVersion(quint64 vsn);
  static QDateTime endFor(QDateTime start, TimeScale scl);
signals:
  void needImage(quint64, QSize);
public slots:
  void rescan();
  void updateImage(quint64, QSize, QImage);
  void setTimeRange(QDateTime t0, TimeScale scl);
  void setArrangement(Arrangement arr);
  void setTileSize(int pix);
  void setRowWidth(int pix);
  void expand();
  void collapse();
  void expandAll();
  void collapseAll();
  /* I have no idea yet how to deal with change in photo counts. Under
     what circumstances should I change whether I split into
     children? */
  void showHeader();
  void hideHeader();
private:
  void clearContents();
  void rebuildContents();
  void rebuildContentsWithSubstrips();
  void rebuildContentsDirectly();
  void relayout();
  int countInRange(QDateTime t0, QDateTime t1) const;
  QList<quint64> versionsInRange(QDateTime t0, QDateTime t1) const;
private:
  PhotoDB db;
  QDateTime d0;
  TimeScale scl;
  Arrangement arr;
  int tilesize;
  int rowwidth;
  bool showheader;
  bool expanded;
  QList<QGraphicsItem *> orderedChildren; // ordered by date
  QMap<QDateTime, Filmstrip *> subStrips;
  // some of our children are strips
  QMap<quint64, Slide *> slides; // other children are image slides
};

#endif
