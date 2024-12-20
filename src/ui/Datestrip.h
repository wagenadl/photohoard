// Datestrip.h

#ifndef DATESTRIP_H

#define DATESTRIP_H

#include "Strip.h"
#include <QTimer>
#include <QPointer>

class Datestrip: public Strip {
  Q_OBJECT;
public:
  Datestrip(class SessionDB *db, QGraphicsItem *parent);
  virtual ~Datestrip();
public:
  virtual QRectF subBoundingRect() const; // children
  virtual Strip *stripByDate(QDateTime t0, TimeScale scl);
  virtual Strip *stripByDate(QDateTime t0); // depth first
  virtual Strip *stripByFolder(QString path);
  virtual class Slide *slideByVersion(quint64 vsn);
  Strip::TimeScale subScale() const;
  virtual quint64 versionAt(quint64 vsn, QPoint dcr);
  virtual quint64 firstExpandedVersion();
  virtual quint64 lastExpandedVersion();
  virtual Strip *firstExpandedStrip();
  virtual Strip *lastExpandedStrip();
  virtual bool isSingleton() const override;
public slots:
  virtual void setArrangement(Arrangement arr);
  virtual void setTileSize(int pix);
  virtual void setRowWidth(int pix);
  virtual void expand();
  virtual void collapse();
  virtual void expandAll();
  virtual void block();
  virtual void unblock();
protected slots:
  virtual void relayout();
  void convertStrip(QDateTime);
protected:
  virtual void clearContents();
  virtual void rebuildContents();
  void rebuildByDate();
  void rebuildByFolder();
  Strip *newSubstrip(QDateTime t, Strip::TimeScale subs);
  Strip *newStrip(bool indirect, bool protectoverfill);
  int stripNumberContaining(quint64 vsn);
private:
  Strip *rightMostSubstrip() const;
  Strip *bottomMostSubstrip() const;
  void findBottomRight() const;
protected:
  QMap<QDateTime, Strip *> dateMap;
  QMap<QString, Strip *> folderMap;
  class Strip *thisFolderStrip; // is a slide strip
  QList<Strip *> stripOrder;
  bool mustRebuild;
  bool mustRelayout;
  int rebuilding;
  mutable QPointer<Strip> rightmostsub, bottommostsub;
  QRectF oldbb;
};

#endif
