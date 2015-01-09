// FilmView.h

#ifndef FILMVIEW_H

#define FILMVIEW_H

#include <QGraphicsView>
#include "Strip.h"

class FilmView: public QGraphicsView {
  Q_OBJECT;
public:
  FilmView(class PhotoDB const &db, QWidget *parent=0);
  virtual ~FilmView();
  class FilmScene *scene() { return scene_; }
  class Datestrip *root() { return strip; }
signals:
  void needImage(quint64, QSize);
  void pressed(quint64, Qt::MouseButton, Qt::KeyboardModifiers);
  void clicked(quint64, Qt::MouseButton, Qt::KeyboardModifiers);
  void doubleClicked(quint64, Qt::MouseButton, Qt::KeyboardModifiers);
public slots:
  void setTileSize(int pix);
  void setArrangement(Strip::Arrangement);
  void updateImage(quint64, QSize, QImage);
  void rescan();
protected:
  virtual void resizeEvent(QResizeEvent *) override;
protected slots:
  void stripResized();
protected:
  void setScrollbarPolicies();
  void recalcSizes();
private:
  class FilmScene *scene_;
  class Datestrip *strip;
};

#endif
