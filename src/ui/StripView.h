// StripView.h

#ifndef STRIPVIEW_H

#define STRIPVIEW_H

#include <QGraphicsView>
#include "Strip.h"
#include "SessionDB.h"
#include "ColorLabelBar.h"
#include "Action.h"

class StripView: public QGraphicsView {
  Q_OBJECT;
public:
  StripView(SessionDB *db, QWidget *parent=0);
  virtual ~StripView();
  class StripScene *scene();
  class Datestrip const *strip() const;
  class Datestrip *strip();
  Strip::Organization organization() const;
  int tileSize() const;
  int idealSize() const;
  int idealSize(Strip::Arrangement) const;
  Actions const &actions() const;
signals:
  void needImage(quint64, QSize);
  void pressed(quint64, Qt::MouseButton, Qt::KeyboardModifiers);
  void clicked(quint64, Qt::MouseButton, Qt::KeyboardModifiers);
  void doubleClicked(quint64, Qt::MouseButton, Qt::KeyboardModifiers);
  void idealSizeChanged(); // only emitted if caused by key press
  void dragStarted(quint64);
public slots:
  void centerOn(QPointF);
  void toggleOrganization();
  void scrollTo(quint64);
  void scrollToCurrent();
  void scrollIfNeeded();
  void setTileSize(int pix);
  void setArrangement(Strip::Arrangement);
  void updateImage(quint64 vsn, Image16 img, bool chgd);
  void quickRotate(quint64, int dphi);
  void rescan();
protected:
  virtual void resizeEvent(QResizeEvent *) override;
  virtual void keyPressEvent(QKeyEvent *) override;
  virtual void enterEvent(QEvent *) override;
  virtual void mousePressEvent(QMouseEvent *) override;
  virtual void dragEnterEvent(QDragEnterEvent *) override;
  virtual void dragMoveEvent(QDragMoveEvent *) override;
  virtual void dragLeaveEvent(QDragLeaveEvent *) override;
  virtual void dropEvent(QDropEvent *) override;
protected slots:
  void stripResized();
protected:
  void setScrollbarPolicies();
  void recalcSizes();
  void placeAndConnect(class Strip *strip);
  void makeActions();
  void startDragScroll(QPoint p0);
private:
  SessionDB *db;
  bool useFolders;
  class StripScene *dateScene, *folderScene;
  class Datestrip *dateStrip, *folderStrip;
  int tilesize;
  Actions acts;
};

#endif
