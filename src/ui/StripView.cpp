// StripView.cpp

#include "StripView.h"
#include "StripScene.h"
#include "Datestrip.h"
#include "PDebug.h"
#include "Slide.h"
#include <QKeyEvent>

StripView::StripView(PhotoDB *db, QWidget *parent):
  QGraphicsView(parent), db(db) {
  tilesize = 96;

  useFolders = false;

  dateScene = new StripScene(db, this);
  setScene(dateScene);
  dateStrip = new Datestrip(db, 0);
  setArrangement(Datestrip::Arrangement::Grid);
  dateStrip->setTileSize(tilesize);
  dateStrip->setTimeRange(QDateTime(), Datestrip::TimeScale::Eternity);
  dateScene->addItem(dateStrip);
  placeAndConnect(dateStrip);

  folderScene = new StripScene(db, this);
  folderStrip = new Datestrip(db, 0);
  setArrangement(Datestrip::Arrangement::Grid);
  folderStrip->setTileSize(tilesize);
  folderStrip->setFolder("/"); /// hmmm.
  folderScene->addItem(folderStrip);
  placeAndConnect(folderStrip);

  dateStrip->block();
  dateStrip->expand();
  QSqlQuery q = db->query("select d0, scl from expanded order by scl");
  while (q.next()) {
    QDateTime d0 = q.value(0).toDateTime();
    Strip::TimeScale scl = Strip::TimeScale(q.value(1).toInt());
    Strip *s = dateStrip->stripByDate(d0, scl);
    if (s) {
      s->block();
      s->expand();
    }
  }
  q = db->query("select d0, scl from expanded order by scl desc");
  while (q.next()) {
    QDateTime d0 = q.value(0).toDateTime();
    Strip::TimeScale scl = Strip::TimeScale(q.value(1).toInt());
    Strip *s = dateStrip->stripByDate(d0, scl);
    if (s) 
      s->unblock();
  }
  q.finish();
  dateStrip->unblock();

  folderStrip->block();
  folderStrip->expand();
  q = db->query("select path from expandedfolders order by path");
  while (q.next()) {
    QString path = q.value(0).toString();
    Strip *s = folderStrip->stripByFolder(path);
    if (s) {
      s->block();
      s->expand();
    }
  }
  q = db->query("select path from expandedfolders order by path desc");
  while (q.next()) {
    QString path = q.value(0).toString();
    Strip *s = folderStrip->stripByFolder(path);
    if (s) 
      s->unblock();
  }
  q.finish();
  folderStrip->unblock();
  
  stripResized();
}

void StripView::placeAndConnect(Strip *strip) {
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

StripView::~StripView() {
}

void StripView::setArrangement(Strip::Arrangement ar) {
  strip()->setArrangement(ar);
  setScrollbarPolicies();
  recalcSizes();
}

void StripView::setScrollbarPolicies() {
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

void StripView::stripResized() {
  QRectF r = strip()->netBoundingRect();
  r |= QRectF(QPointF(0, 0), viewport()->size());
  scene()->setSceneRect(r);
  pDebug() << "stripResized";
  update();
}

void StripView::resizeEvent(QResizeEvent *) {
  recalcSizes();
  scrollToCurrent();
}

void StripView::recalcSizes() {
  switch (strip()->arrangement()) {
  case Datestrip::Arrangement::Horizontal:
    setTileSize(viewport()->height());
    break;
  case Datestrip::Arrangement::Vertical:
    setTileSize(viewport()->width());
    break;
  case Datestrip::Arrangement::Grid:
    strip()->setRowWidth(viewport()->width());
    break;
  }
}

void StripView::updateImage(quint64 id, Image16 img) {
  scene()->updateImage(id, img);
}

void StripView::rescan() {
  dateStrip->rescan();
}

void StripView::setTileSize(int pix) {
  if (pix<50)
    pix = 50;
  else if (pix>1024)
    pix = 1024;
  tilesize = pix;
  strip()->setTileSize(pix);
}

void StripView::scrollTo(quint64 vsn) {
  Slide *s = strip()->slideByVersion(vsn);
  if (s)
    centerOn(s->sceneBoundingRect().center());
}

void StripView::scrollToCurrent() {
  quint64 c = current();
  if (c)
    scrollTo(c);
}

void StripView::scrollIfNeeded() {
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

quint64 StripView::current() {
  return db->simpleQuery("select * from current").toULongLong();
}

int StripView::idealSize() const {
  return idealSize(strip()->arrangement());
}

int StripView::idealSize(Strip::Arrangement arr) const {
  switch (arr) {
  case Strip::Arrangement::Horizontal:
    return tilesize + height()-viewport()->height();
  case Strip::Arrangement::Vertical:
    return tilesize + width()-viewport()->width();
  default:
    break;
  }
  return tilesize * 2 + width()-viewport()->width() + 4
    + 5*Strip::labelHeight(tileSize());
}

void StripView::keyPressEvent(QKeyEvent *e) {
  switch (e->key()) {
  case Qt::Key_Minus:
    setTileSize(tilesize*8/10);
    emit idealSizeChanged();
    break;
  case Qt::Key_Plus: case Qt::Key_Equal:
    setTileSize(tilesize*10/8);
    emit idealSizeChanged();
    break;
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

StripScene *StripView::scene() {
  return useFolders ? folderScene : dateScene;
}

Datestrip *StripView::strip() {
  return useFolders ? folderStrip : dateStrip;
}

Datestrip const *StripView::strip() const {
  return useFolders ? folderStrip : dateStrip;
}

void StripView::enterEvent(QEvent *) {
  setFocus();
}

Strip::Organization StripView::organization() const {
  return useFolders ? Strip::Organization::ByFolder
    : Strip::Organization::ByDate;
}

void StripView::toggleOrganization() {
  Datestrip *oldstrip = strip();
  useFolders = !useFolders;
  Datestrip *newstrip = strip();
  newstrip->setArrangement(oldstrip->arrangement());
  newstrip->setTileSize(tilesize);
  newstrip->setRowWidth(oldstrip->rowWidth());
  setScene(scene());
  rescan();
  scrollToCurrent();
}

int StripView::tileSize() const {
  return tilesize;
}