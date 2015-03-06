// FilmView.cpp

#include "FilmView.h"
#include "FilmScene.h"
#include "Datestrip.h"
#include <QDebug>
#include "Slide.h"

FilmView::FilmView(PhotoDB const &db, QWidget *parent):
  QGraphicsView(parent) {
  scene_ = new FilmScene(db, this);
  setScene(scene_);
  strip = new Datestrip(db, 0);
  setArrangement(Datestrip::Arrangement::Grid);
  strip->setTileSize(80); //width()); // minus scrollbar...
  strip->setTimeRange(QDateTime(),
		      Datestrip::TimeScale::Eternity);
  scene_->addItem(strip);
  strip->setPos(0, 0);
  connect(strip, SIGNAL(resized()),
	  this, SLOT(stripResized()));
  connect(strip, SIGNAL(needImage(quint64, PSize)),
	  this, SIGNAL(needImage(quint64, PSize)));
  connect(strip,
          SIGNAL(pressed(quint64, Qt::MouseButton, Qt::KeyboardModifiers)),
	  this,
          SIGNAL(pressed(quint64, Qt::MouseButton, Qt::KeyboardModifiers)));
  connect(strip,
          SIGNAL(clicked(quint64, Qt::MouseButton, Qt::KeyboardModifiers)),
	  this,
          SIGNAL(clicked(quint64, Qt::MouseButton, Qt::KeyboardModifiers)));
  connect(strip,
          SIGNAL(doubleClicked(quint64,
                               Qt::MouseButton, Qt::KeyboardModifiers)),
	  this,
          SIGNAL(doubleClicked(quint64,
                               Qt::MouseButton, Qt::KeyboardModifiers)));

  PhotoDB dbx(db);
  dbx.beginAndLock();
  QSqlQuery q = dbx.query("select d0, scl from expanded order by scl");
  bool any = false;
  while (q.next()) {
    QDateTime d0 = q.value(0).toDateTime();
    Strip::TimeScale scl = Strip::TimeScale(q.value(1).toInt());
    Strip *s = strip->stripByDate(d0, scl);
    if (s) {
      s->expand();
      any = true;
    }
  }
  if (!any)
    strip->expand();
  dbx.commitAndUnlock();
  
  stripResized();
}

FilmView::~FilmView() {
}

void FilmView::setArrangement(Strip::Arrangement ar) {
  strip->setArrangement(ar);
  setScrollbarPolicies();
  recalcSizes();
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
  scene_->setSceneRect(r);
}

void FilmView::resizeEvent(QResizeEvent *) {
  recalcSizes();
}

void FilmView::recalcSizes() {
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

void FilmView::updateImage(quint64 id, Image16 img) {
  scene_->updateImage(id, img);
}

void FilmView::rescan() {
  strip->rescan();
}

void FilmView::setTileSize(int pix) {
  strip->setTileSize(pix);
}

void FilmView::scrollTo(quint64 vsn) {
  Slide *s = root()->slideByVersion(vsn);
  qDebug() << "scrollTo" << vsn << s;
  if (s)
    centerOn(s->sceneBoundingRect().center());
}

  
