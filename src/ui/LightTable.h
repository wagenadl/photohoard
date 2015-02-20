// LightTable.h

#ifndef LIGHTTABLE_H

#define LIGHTTABLE_H

#include <QSplitter>
#include "PhotoDB.h"
#include "Strip.h"
#include "LayoutBar.h"
#include "ColorLabelBar.h"
#include "FilterBar.h"

class LightTable: public QSplitter {
  Q_OBJECT;
public:
  LightTable(PhotoDB const &db, QWidget *parent=0);
  virtual ~LightTable();
public slots:
  void setLayout(LayoutBar::Action ar);
  void slidePress(quint64 id, Qt::MouseButton, Qt::KeyboardModifiers);
  void select(quint64 id, Qt::KeyboardModifiers);
  void updateImage(quint64, QSize, Image16);
  void rescan();
  void setColorLabel(ColorLabelBar::Action);
  void filterAction(FilterBar::Action);
  void clearSelection();
  void bgPress(Qt::MouseButton, Qt::KeyboardModifiers);
  void scrollToCurrent();
signals:
  void needImage(quint64, QSize);
  void newCurrent(quint64);
private slots:
  void requestLargerImage();
protected:
  void updateSlide(quint64 id);
protected:
  PhotoDB db;
  class Selection *selection;
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
