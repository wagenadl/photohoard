// LightTable.cpp

#include "LightTable.h"
#include "NoResult.h"
#include "FilmView.h"
#include "SlideView.h"

LightTable::LightTable(PhotoDB const &db, QWidget *parent):
  QSplitter(parent), db(db) {
  setObjectName("LightTable");
  film = new FilmView(db);
  addWidget(film);
  slide = new SlideView();
  addWidget(slide);
  arr = Strip::Arrangement::Vertical;
  showmax = false;

  connect(film, SIGNAL(needImage(quint64, QSize)),
	  this, SIGNAL(needImage(quint64, QSize)));
  connect(film, SIGNAL(pressed(quint64)),
	  this, SLOT(select(quint64)));
  connect(slide, SIGNAL(needLargerImage()),
	  this, SLOT(select()));
}

LightTable::~LightTable() {
}

void LightTable::setArrangement(Strip::Arrangement ar) {
  arr = ar;
  switch (arr) {
  case Strip::Arrangement::Vertical:
    setOrientation(Qt::Vertical);
    film->show();
    slide->show();
    break;
  case Strip::Arrangement::Horizontal:
    setOrientation(Qt::Horizontal);
    film->show();
    slide->show();
    break;
  case Strip::Arrangement::Grid:
    film->show();
    slide->hide();
    break;
  }
}

void LightTable::maximize() {
  showmax = true;
  film->hide();
  slide->show();
}

void LightTable::unMaximize() {
  showmax = false;
  setArrangement(arr);
}

void LightTable::select(quint64 i) {
  if (i==id)
    return;

  if (i!=0) {
    id = i;
    newImage = true;
    emit selected(id);
  }
  
  if (showmax || arr!=Strip::Arrangement::Grid) 
    emit needImage(id, slide->desiredSize());
}

void LightTable::updateImage(quint64 i, QSize s, QImage img) {
  film->updateImage(i, s, img);

  if (i!=id)
    return;

  if (newImage) {
    QSqlQuery q(*db);
    q.prepare("select photos.width, photos.height"
	      " from photos inner join versions"
	      " on photos.id=versions.photo"
	      " where versions.id==:i"
	      " limit 1");
    q.bindValue(":i", id);
    if (!q.exec())
      throw q;
    if (!q.next())
      throw NoResult();
    
    slide->newImage(QSize(q.value(0).toInt(), q.value(1).toInt()));
    newImage = false;
  }

  slide->updateImage(img);
}

void LightTable::rescan() {
  film->rescan();
}
