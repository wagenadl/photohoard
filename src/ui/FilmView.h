// FilmView.h

#ifndef FILMVIEW_H

#define FILMVIEW_H

#include <QGraphicsView>
#include "Strip.h"
#include "PhotoDB.h"

class FilmView: public QGraphicsView {
  Q_OBJECT;
public:
  FilmView(PhotoDB *db, QWidget *parent=0);
  virtual ~FilmView();
  class FilmScene *scene();
  class Datestrip *strip();
  Strip::Organization organization() const;
signals:
  void needImage(quint64, QSize);
  void pressed(quint64, Qt::MouseButton, Qt::KeyboardModifiers);
  void clicked(quint64, Qt::MouseButton, Qt::KeyboardModifiers);
  void doubleClicked(quint64, Qt::MouseButton, Qt::KeyboardModifiers);
public slots:
  void toggleOrganization();
  void scrollTo(quint64);
  void scrollToCurrent();
  void scrollIfNeeded();
  void setTileSize(int pix);
  void setArrangement(Strip::Arrangement);
  void updateImage(quint64, Image16);
  void rescan();
protected:
  virtual void resizeEvent(QResizeEvent *) override;
  virtual void keyPressEvent(QKeyEvent *) override;
  virtual void enterEvent(QEvent *) override;
protected slots:
  void stripResized();
protected:
  void setScrollbarPolicies();
  void recalcSizes();
  quint64 current();
  void placeAndConnect(class Strip *strip);
private:
  PhotoDB *db;
  bool useFolders;
  class FilmScene *dateScene, *folderScene;
  class Datestrip *dateStrip, *folderStrip;
};

#endif
