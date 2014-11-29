// LightTable.h

#ifndef LIGHTTABLE_H

#define LIGHTTABLE_H

#include <QSplitter>
#include "Filmstrip.h"
#include "PhotoDB.h"

class LightTable: public QSplitter {
  Q_OBJECT;
public:
  LightTable(PhotoDB const &db, QWidget *parent=0);
  virtual ~LightTable();
public slots:
  void setArrangement(Filmstrip::Arrangement ar);
  void maximize();
  void unMaximize();
  void select(quint64 id=0);
  void updateImage(quint64, QSize, QImage);
  void rescan();
signals:
  void needImage(quint64, QSize);
  void selected(quint64);
protected:
  PhotoDB db;
  class FilmView *film;
  class SlideView *slide;
  bool showmax;
  Filmstrip::Arrangement arr;
  quint64 id;
  bool newImage;
};

#endif
