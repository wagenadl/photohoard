// Datestrip.h

#ifndef DATESTRIP_H

#define DATESTRIP_H

#include "Strip.h"

class Datestrip: public Strip {
public:
  Datestrip(PhotoDB const &db, QGraphicsItem *parent);
  virtual ~Datestrip();
public:
  virtual QRectF subBoundingRect() const; // children
  virtual class Slide *slideByVersion(quint64 vsn) const;
  Strip::TimeScale subScale() const;
public slots:
  virtual void setArrangement(Arrangement arr);
  virtual void setTileSize(int pix);
  virtual void setRowWidth(int pix);
  virtual void expand();
  virtual void collapse();
  virtual void expandAll();
  virtual void collapseAll();
protected slots:
  virtual void relayout();
protected:
  virtual void clearContents();
  virtual void rebuildContents();
protected:
  QMap<QDateTime, Strip *> stripMap;
  QList<Strip *> stripOrder;
  bool mustRebuild;
  bool mustRelayout;
  bool rebuilding;
};

#endif
