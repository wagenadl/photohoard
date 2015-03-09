// Datestrip.h

#ifndef DATESTRIP_H

#define DATESTRIP_H

#include "Strip.h"
#include <QTimer>

class Datestrip: public Strip {
  Q_OBJECT;
public:
  Datestrip(PhotoDB const &db, QGraphicsItem *parent);
  virtual ~Datestrip();
public:
  virtual QRectF subBoundingRect() const; // children
  virtual Strip *stripByDate(QDateTime t0, TimeScale scl);
  virtual class Slide *slideByVersion(quint64 vsn);
  Strip::TimeScale subScale() const;
  virtual quint64 versionAt(quint64 vsn, QPoint dcr);
  virtual quint64 firstExpandedVersion();
  virtual quint64 lastExpandedVersion();
  virtual Strip *firstExpandedStrip();
  virtual Strip *lastExpandedStrip();
public slots:
  virtual void setArrangement(Arrangement arr);
  virtual void setTileSize(int pix);
  virtual void setRowWidth(int pix);
  virtual void expand();
  virtual void collapse();
  virtual void expandAll();
protected slots:
  virtual void relayout();
  virtual void convertStrip(QDateTime);
protected:
  virtual void clearContents();
  virtual void rebuildContents();
  Strip *newSubstrip(QDateTime t, Strip::TimeScale subs);
  int stripNumberContaining(quint64 vsn);
protected:
  QMap<QDateTime, Strip *> stripMap;
  QList<Strip *> stripOrder;
  bool mustRebuild;
  bool mustRelayout;
  int rebuilding;
};

#endif
