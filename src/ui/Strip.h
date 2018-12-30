// Strip.h

#ifndef STRIP_H

#define STRIP_H

#include <QGraphicsObject>
#include "SessionDB.h"
#include <QDateTime>
#include "Image16.h"

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
  enum class Organization {
    ByDate,
    ByFolder,
  };
public:
  Strip(SessionDB *db, QGraphicsItem *parent);
  virtual ~Strip();
  QDateTime startDateTime() const;
  QDateTime endDateTime() const;
  TimeScale timeScale() const;
  Arrangement arrangement() const { return arr; }
  bool isExpanded() const { return expanded; }
  int subRowWidth(int pix) const;
  int tileSize() const { return tilesize; }
  int rowWidth() const { return rowwidth; }
  bool hasTopLabel() const;
  virtual Strip *stripByDate(QDateTime t0, TimeScale scl);
  virtual Strip *stripByDate(QDateTime t0);
  virtual Strip *stripByFolder(QString path);
  virtual class Slide *slideByVersion(quint64 vsn);
  SessionDB *database() { return db; }
  virtual quint64 versionLeftOf(quint64 vsn); // returns 0 if not found
  virtual quint64 versionRightOf(quint64 vsn);
  virtual quint64 versionAbove(quint64 vsn);
  virtual quint64 versionBelow(quint64 vsn);
  virtual quint64 versionAt(quint64 vsn, QPoint dcr)=0;
  // dcr must be one of (±1,0) or (0,±1).
  virtual quint64 versionBefore(quint64 vsn);
  virtual quint64 versionAfter(quint64 vsn);
  virtual quint64 firstExpandedVersion()=0;
  virtual quint64 lastExpandedVersion()=0;
  virtual Strip *firstExpandedStrip()=0;
  virtual Strip *lastExpandedStrip()=0;
  Strip *parentStrip() const;
public slots:
  void makeHeaderless();
  void rescan();
  void updateImage(quint64 id, Image16 img, bool chgd);
  void updateHeader(Image16 img, bool chgd);
  void updateHeaderRotation(int dphi);
  void setTimeRange(QDateTime t0, TimeScale scl);
  void setFolder(QString pathname);
  void setDisplayName(QString leaf);
  virtual void setArrangement(Arrangement arr);
  virtual void setTileSize(int pix);
  virtual void setRowWidth(int pix);
  virtual void expand();
  void expandWithParents();
  virtual void collapse();
  virtual void expandAll();
  virtual void block() {}
  virtual void unblock() {}
signals:
  void pleaseRecenter(QPointF);
  void needImage(quint64, QSize);
  void resized();
  void pressed(quint64, Qt::MouseButton, Qt::KeyboardModifiers);
  void clicked(quint64, Qt::MouseButton, Qt::KeyboardModifiers);
  void doubleClicked(quint64, Qt::MouseButton, Qt::KeyboardModifiers);
  void dragStarted(quint64);
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
  QString longLabelFor(QDateTime t0, TimeScale scl);
  static int labelHeight(int tilesize);
  void requestImage(quint64);
  virtual bool isSingleton() const=0;
protected:
  virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *) override;
  virtual void mousePressEvent(QGraphicsSceneMouseEvent *) override;
  virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *) override;
protected slots:
  virtual void relayout();
protected:
  virtual void rebuildToolTip();
  virtual void clearContents();
  virtual void rebuildContents();
  void setHeaderID(quint64);
  void paintHeaderText(QPainter *painter, QRectF r);  
  void paintHeaderImage(QPainter *painter, QRectF r);
  void paintExpandedHeaderBox(QPainter *painter, QRectF r, QColor bg);
  void paintCollapsedHeaderBox(QPainter *painter, QRectF r, QColor bg);
protected:
  int subHeight() const;
  void recalcLabelRect();
  void toggleSelection();
  static QPixmap const &dashPattern(QColor);
protected:
  SessionDB *db;
  Organization org;
  QDateTime d0;
  TimeScale scl;
  QString pathname, leafname;
  Arrangement arr;
  int tilesize;
  int rowwidth;
  bool expanded;
  mutable int subheight;
  quint64 headerid;
  Image16 headerimg;
  QPixmap headerpm;
  QRectF labelRect;
  bool hasheader;
};

#endif
