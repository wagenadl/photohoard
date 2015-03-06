// LightTable.h

#ifndef LIGHTTABLE_H

#define LIGHTTABLE_H

#include <QSplitter>
#include <QPointer>

#include "PhotoDB.h"
#include "Strip.h"
#include "LayoutBar.h"
#include "ColorLabelBar.h"
#include "FilterBar.h"
#include "LiveAdjuster.h"

class LightTable: public QSplitter {
  Q_OBJECT;
public:
  LightTable(PhotoDB const &db, LiveAdjuster *adjuster,
             QWidget *parent=0);
  virtual ~LightTable();
  quint64 current() const { return id; }
  QSize displaySize() const;
public slots:
  void setLayout(LayoutBar::Action ar);
  void slidePress(quint64 id, Qt::MouseButton, Qt::KeyboardModifiers);
  void select(quint64 id, Qt::KeyboardModifiers);
  void updateImage(quint64, Image16);
  void rescan();
  void setColorLabel(ColorLabelBar::Action);
  void filterAction(FilterBar::Action);
  void clearSelection();
  void bgPress(Qt::MouseButton, Qt::KeyboardModifiers);
  void scrollToCurrent();
signals:
  void needImage(quint64, QSize);
  void newCurrent(quint64);
  void newSlideSize(QSize);
private slots:
  void requestLargerImage();
  void updateAdjusted(Image16, quint64);
protected:
  void updateSlide(quint64 id);
protected:
  PhotoDB db;
  QPointer<LiveAdjuster> adjuster; // we do not own
  class Selection *selection;
  class FilmView *film;
  class SlideView *slide;
  bool showmax;
  LayoutBar::Action lastlay, lay;
  quint64 id;
  int tilesize;
  int lastgridsize;
};

#endif
