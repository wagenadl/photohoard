// FilmView.cpp

#include "FilmView.h"

#include "Filmstrip.h"

FilmView::FilmView(PhotoDB const &db, QWidget *parent):
  QGraphicsView(parent) {
  scene = new QGraphicsScene(this);
  setScene(scene);
  strip = new Filmstrip(db, 0);
  strip->setArrangement(Filmstrip::Arrangement::Vertical);
  setScrollbarPolicies();
  strip->setTileSize(width()); // minus scrollbar...
  strip->setTimeRange(QDateTime(QDate(2010,1,1), QTime(0,0,0)),
		      Filmstrip::TimeScale::Decade);
  scene->addItem(strip);
  connect(strip, SIGNAL(resized()),
	  this, SLOT(stripResized()));
  connect(strip, SIGNAL(needImage(quint64, QSize)),
	  this, SIGNAL(needImage(quint64, QSize)));
  connect(strip, SIGNAL(pressed(quint64)),
	  this, SIGNAL(pressed(quint64)));
  connect(strip, SIGNAL(clicked(quint64)),
	  this, SIGNAL(clicked(quint64)));
  connect(strip, SIGNAL(doubleClicked(quint64)),
	  this, SIGNAL(doubleClicked(quint64)));
  stripResized();
}

FilmView::~FilmView() {
}

void FilmView::setScrollbarPolicies() {
  switch (strip->arrangement()) {
  case Filmstrip::Arrangement::Horizontal:
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    break;
  case Filmstrip::Arrangement::Vertical:
  case Filmstrip::Arrangement::Grid:
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    break;
  }
}

void FilmView::stripResized() {
  scene->setSceneRect(strip->netBoundingRect());
}

void FilmView::resizeEvent(QResizeEvent *) {
  switch (strip->arrangement()) {
  case Filmstrip::Arrangement::Horizontal:
    strip->setTileSize(viewport()->height());
    break;
  case Filmstrip::Arrangement::Vertical:
    strip->setTileSize(viewport()->width());
    break;
  case Filmstrip::Arrangement::Grid:
    strip->setRowWidth(viewport()->width());
    break;
  }
}

void FilmView::updateImage(quint64 id, QSize s, QImage img) {
  strip->updateImage(id, s, img);
}