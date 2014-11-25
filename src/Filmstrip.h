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
  Filmstrip(PhotoDB const &db, QGraphicsItem *parent);
  virtual ~Filmstrip();
  QDateTime startDateTime() const;
  QDateTime endDateTime() const;
  TimeScale timeScale() const;
  virtual QRectF boundingRect() const override; // just us, i.e., our header
  virtual QRectF netBoundingRect() const; // us and children
  virtual QRectF subBoundingRect() const; // children
  virtual void paint(QPainter *painter,
		     const QStyleOptionGraphicsItem *option,
		     QWidget *widget=0) override;
  Filmstrip *descendentByDate(QDateTime d);
  /* Returns the strip that would directly host a picture with given date.
     That might be us or a (grand)child. It can also be NULL if the date
     is outside our range. */
  class Slide *slideByVersion(quint64 vsn);
  static QDateTime startFor(QDateTime d, TimeScale scl);
  static QDateTime endFor(QDateTime start, TimeScale scl);
  static QString labelFor(QDateTime t0, TimeScale scl);
  Arrangement arrangement() const { return arr; }
  static int labelHeight(int tilesize);
  bool isExpanded() const { return expanded; }
  int subRowWidth(int pix) const;
  bool hasTopLabel() const;
protected:
  virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *) override;
  virtual void mousePressEvent(QGraphicsSceneMouseEvent *) override;
  virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *) override;
signals:
  void needImage(quint64, QSize);
  void resized();
  void pressed(quint64);
  void clicked(quint64);
  void doubleClicked(quint64);
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
public: // for slide
  void requestImage(quint64);
  void slidePressed(quint64);
  void slideClicked(quint64);
  void slideDoubleClicked(quint64);		    
private slots:
  void relayout();
private:
  void clearContents();
  void rebuildContents();
  void rebuildContentsWithSubstrips();
  void rebuildContentsDirectly();
  int countInRange(QDateTime t0, QDateTime t1) const;
  bool anyInRange(QDateTime t0, QDateTime t1) const;
  QList<quint64> versionsInRange(QDateTime t0, QDateTime t1) const;
  QDateTime firstDateIn(QDateTime t0, QDateTime t1) const;
  QDateTime lastDateIn(QDateTime t0, QDateTime t1) const;
private:
  PhotoDB db;
  QDateTime d0;
  TimeScale scl;
  Arrangement arr;
  int tilesize;
  int rowwidth;
  bool showheader;
  bool expanded;
  bool rebuilding;
  bool needsRebuild;
  QList<QGraphicsItem *> orderedChildren; // ordered by date
  // Either, all our children are substripts, or all are slides.
  QMap<QDateTime, Filmstrip *> subStrips;
  QMap<quint64, Slide *> slides;
};

#endif
