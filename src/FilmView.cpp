// FilmView.cpp

#include "FilmView.h"
#include "FilmScene.h"
#include "Datestrip.h"

FilmView::FilmView(PhotoDB const &db, QWidget *parent):
  QGraphicsView(parent) {
  scene = new FilmScene(db, this);
  setScene(scene);
  strip = new Datestrip(db, 0);
  strip->setArrangement(Datestrip::Arrangement::Grid);
  setScrollbarPolicies();
  strip->setTileSize(80); //width()); // minus scrollbar...
  strip->setTimeRange(QDateTime(),
		      Datestrip::TimeScale::Eternity);
  scene->addItem(strip);
  strip->setPos(0, 0);
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
  strip->expand();
  stripResized();
}

FilmView::~FilmView() {
}

void FilmView::setScrollbarPolicies() {
  switch (strip->arrangement()) {
  case Datestrip::Arrangement::Horizontal:
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    break;
  case Datestrip::Arrangement::Vertical:
  case Datestrip::Arrangement::Grid:
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    break;
  }
}

void FilmView::stripResized() {
  QRectF r = strip->netBoundingRect();
  r |= QRectF(QPointF(0, 0), viewport()->size());
  scene->setSceneRect(r);
}

void FilmView::resizeEvent(QResizeEvent *) {
  switch (strip->arrangement()) {
  case Datestrip::Arrangement::Horizontal:
    strip->setTileSize(viewport()->height());
    break;
  case Datestrip::Arrangement::Vertical:
    strip->setTileSize(viewport()->width());
    break;
  case Datestrip::Arrangement::Grid:
    strip->setRowWidth(viewport()->width());
    break;
  }
}

void FilmView::updateImage(quint64 id, QSize, QImage img) {
  scene->updateImage(id, img);
}

void FilmView::rescan() {
  strip->rescan();
}
