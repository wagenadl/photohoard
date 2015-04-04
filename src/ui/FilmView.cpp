// FilmView.cpp

#include "FilmView.h"
#include "FilmScene.h"
#include "Datestrip.h"
#include "PDebug.h"
#include "Slide.h"
#include <QKeyEvent>

FilmView::FilmView(PhotoDB const &db, QWidget *parent):
  QGraphicsView(parent), db(db) {
  dateScene = new FilmScene(db, this);
  setScene(dateScene);
  dateStrip = new Datestrip(db, 0);
  setArrangement(Datestrip::Arrangement::Grid);
  dateStrip->setTileSize(80);
  dateStrip->setTimeRange(QDateTime(), Datestrip::TimeScale::Eternity);
  dateScene->addItem(dateStrip);
  placeAndConnect(dateStrip);

  PhotoDB dbx(db);
  dbx.beginAndLock();
  QSqlQuery q = dbx.query("select d0, scl from expanded order by scl");
  bool any = false;
  while (q.next()) {
    QDateTime d0 = q.value(0).toDateTime();
    Strip::TimeScale scl = Strip::TimeScale(q.value(1).toInt());
    Strip *s = dateStrip->stripByDate(d0, scl);
    if (s) {
      s->expand();
      any = true;
    }
  }
  if (!any)
    dateStrip->expand();
  dbx.commitAndUnlock();
  
  stripResized();
}

void FilmView::placeAndConnect(Strip *strip) {
  strip->setPos(0, 0);
  connect(strip, SIGNAL(resized()),
	  this, SLOT(stripResized()));
  connect(strip, SIGNAL(needImage(quint64, QSize)),
	  this, SIGNAL(needImage(quint64, QSize)));
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
}

FilmView::~FilmView() {
}

void FilmView::setArrangement(Strip::Arrangement ar) {
  strip()->setArrangement(ar);
  setScrollbarPolicies();
  recalcSizes();
}

void FilmView::setScrollbarPolicies() {
  switch (strip()->arrangement()) {
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
  QRectF r = strip()->netBoundingRect();
  r |= QRectF(QPointF(0, 0), viewport()->size());
  scene()->setSceneRect(r);
}

void FilmView::resizeEvent(QResizeEvent *) {
  recalcSizes();
  scrollToCurrent();
}

void FilmView::recalcSizes() {
  switch (strip()->arrangement()) {
  case Datestrip::Arrangement::Horizontal:
    strip()->setTileSize(viewport()->height());
    break;
  case Datestrip::Arrangement::Vertical:
    strip()->setTileSize(viewport()->width());
    break;
  case Datestrip::Arrangement::Grid:
    strip()->setRowWidth(viewport()->width());
    break;
  }
}

void FilmView::updateImage(quint64 id, Image16 img) {
  scene()->updateImage(id, img);
}

void FilmView::rescan() {
  dateStrip->rescan();
}

void FilmView::setTileSize(int pix) {
  strip()->setTileSize(pix);
}

void FilmView::scrollTo(quint64 vsn) {
  Slide *s = strip()->slideByVersion(vsn);
  if (s)
    centerOn(s->sceneBoundingRect().center());
}

void FilmView::scrollToCurrent() {
  quint64 c = current();
  if (c)
    scrollTo(c);
}

void FilmView::scrollIfNeeded() {
  quint64 c = current();
  if (!c)
    return;
  Slide *cs = strip()->slideByVersion(c);
  if (!cs)
    return;
  QRect portRect = viewport()->rect();
  QRectF sceneRect = mapToScene(portRect).boundingRect();
  QRectF itemRect = cs->mapRectFromScene(sceneRect);
  if (!itemRect.contains(cs->boundingRect()))
    scrollTo(c);
}

quint64 FilmView::current() {
  return db.simpleQuery("select * from current").toULongLong();
}

void FilmView::keyPressEvent(QKeyEvent *e) {
  switch (e->key()) {
  case Qt::Key_Up: {
    quint64 v = strip()->versionAbove(current());
    if (v)
      emit pressed(v, Qt::LeftButton, 0); // bit of a hack
    e->accept();
  } break;
  case Qt::Key_Down: {
    quint64 v = strip()->versionBelow(current());
    if (v)
      emit pressed(v, Qt::LeftButton, 0); // bit of a hack
    e->accept();
  } break;
  case Qt::Key_Left: {
    quint64 v = strip()->versionLeftOf(current());
    if (v)
      emit pressed(v, Qt::LeftButton, 0); // bit of a hack
    e->accept();
  } break;
  case Qt::Key_Right: {
    quint64 v = strip()->versionRightOf(current());
    if (v)
      emit pressed(v, Qt::LeftButton, 0); // bit of a hack
    e->accept();
  } break;
  default:
    QGraphicsView::keyPressEvent(e);
    break;
  }
}

FilmScene *FilmView::scene() {
  return dateScene;
}

Datestrip *FilmView::strip() {
  return dateStrip;
}

void FilmView::enterEvent(QEvent *) {
  setFocus();
}
