// LightTable.h

#ifndef LIGHTTABLE_H

#define LIGHTTABLE_H

#include <QSplitter>
#include "PhotoDB.h"
#include "Strip.h"
#include "LayoutBar.h"

class LightTable: public QSplitter {
  Q_OBJECT;
public:
  LightTable(PhotoDB const &db, QWidget *parent=0);
  virtual ~LightTable();
public slots:
  void setLayout(LayoutBar::Action ar);
  void slidePress(quint64 id, Qt::MouseButton, Qt::KeyboardModifiers);
  void select(quint64 id, Qt::KeyboardModifiers);
  void updateImage(quint64, QSize, QImage);
  void rescan();
signals:
  void needImage(quint64, QSize);
  void selected(quint64);
private slots:
  void requestLargerImage();
protected:
  PhotoDB db;
  class FilmView *film;
  class SlideView *slide;
  bool showmax;
  LayoutBar::Action lastlay, lay;
  quint64 id;
  bool newImage;
  int tilesize;
  int lastgridsize;
};

#endif
