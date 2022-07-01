// StripView.cpp

#include "StripView.h"
#include "StripScene.h"
#include "Datestrip.h"
#include "PDebug.h"
#include "Slide.h"
#include <QKeyEvent>
#include <QDrag>
#include <QMimeData>
#include "Settings.h"

StripView::StripView(SessionDB *db, QWidget *parent):
  QGraphicsView(parent), db(db) {
  tilesize = Settings().get("tilesize", 96).toInt();

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
  QList<QDateTime> d0s;
  QList<Strip::TimeScale> scls;
  { DBReadLock lock(db);
    QSqlQuery q = db->constQuery("select d0, scl from expanded order by scl");
    while (q.next()) {
      d0s << q.value(0).toDateTime();
      scls << Strip::TimeScale(q.value(1).toInt());
    }
  }
  for (int n=0; n<d0s.size(); n++) {
    Strip *s = dateStrip->stripByDate(d0s[n], scls[n]);
    if (s) {
      s->block();
      s->expand();
    }
  }

  d0s.clear();
  scls.clear();
  { DBReadLock lock(db);
    QSqlQuery q = db->constQuery("select d0, scl from expanded order by scl desc");
    while (q.next()) {
      d0s << q.value(0).toDateTime();
      scls << Strip::TimeScale(q.value(1).toInt());
    }
  }
  for (int n=0; n<d0s.size(); n++) {
    Strip *s = dateStrip->stripByDate(d0s[n], scls[n]);
    if (s) 
      s->unblock();
  }

  dateStrip->unblock();

  folderStrip->block();
  folderStrip->expand();
  QStringList paths;
  { DBReadLock lock(db);
    QSqlQuery q = db->constQuery("select path from expandedfolders order by path");
    while (q.next()) 
      paths << q.value(0).toString();
  }
  for (QString path: paths) {
    Strip *s = folderStrip->stripByFolder(path);
    if (s) {
      s->block();
      s->expand();
    }
  }

  paths.clear();
  { DBReadLock lock(db);
    QSqlQuery q
      = db->constQuery("select path from expandedfolders order by path desc");
    while (q.next()) 
      paths << q.value(0).toString();
  }
  for (QString path: paths) {
    Strip *s = folderStrip->stripByFolder(path);
    if (s) 
      s->unblock();
  }

  folderStrip->unblock();
  
  stripResized();

  makeActions();
}

void StripView::placeAndConnect(Strip *strip) {
  strip->setPos(0, 0);
  connect(strip, SIGNAL(pleaseRecenter(QPointF)),
          SLOT(centerOn(QPointF)));
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
  connect(strip, SIGNAL(dragStarted(quint64)),
          this, SIGNAL(dragStarted(quint64)));
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
  //  scrollIfNeeded(); //?
  /* This will helpfully cause the current to stay visible during
     filter change, but it also causes much unwanted scrolling.
  */
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

void StripView::updateImage(quint64 id, Image16 img, bool chgd) {
  scene()->updateImage(id, img, chgd);
}

void StripView::quickRotate(quint64 id, int dphi) {
  scene()->quickRotate(id, dphi);
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
  quint64 c = db->current();
  if (c)
    scrollTo(c);
}

void StripView::scrollIfNeeded() {
  quint64 c = db->current();
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

void StripView::makeActions() {
}

void StripView::keyPressEvent(QKeyEvent *e) {
  if (acts.activateIf(e)) {
    e->accept();
    return;
  } else {
    switch (e->key()) {
    case Qt::Key_Left: case Qt::Key_Right:
    case Qt::Key_Up: case Qt::Key_Down:
    case Qt::Key_BracketLeft: case Qt::Key_BracketRight:
    case Qt::Key_Period:
      e->ignore(); // let LightTable handle these
      return;
    default:
      break;
    }
  }
  
  QGraphicsView::keyPressEvent(e); // handles scrolling with PgUp/PgDn
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

Actions const &StripView::actions() const {
  return acts;
}

void StripView::centerOn(QPointF p) {
  QGraphicsView::centerOn(p);
}

void StripView::mousePressEvent(QMouseEvent *e) {
  QGraphicsView::mousePressEvent(e);
  if (e->button()==Qt::LeftButton && !e->isAccepted())
    startDragScroll(e->pos());
}

void StripView::startDragScroll(QPoint p0) {
  QWidget *vp = viewport();
  ASSERT(vp);
  QSize vps = vp->size();
  QPoint center(vps.width()/2, vps.height()/2);
  QPoint inscene = mapToScene(center).toPoint();
  QDrag *drag = new QDrag(this);
  QMimeData *data = new QMimeData;
  QByteArray ar(16, ' ');
  qint32 *d = reinterpret_cast<qint32*>(ar.data());
  d[0] = p0.x();
  d[1] = p0.y();
  d[2] = inscene.x();
  d[3] = inscene.y();
  data->setData("int32/point", ar);
  drag->setMimeData(data);
  QPixmap pm(1, 1);
  pm.fill(QColor(255, 255, 255));
  drag->setDragCursor(pm, Qt::MoveAction); // this does nothing
  drag->setPixmap(pm);
  drag->exec(Qt::MoveAction);
}

void StripView::dragEnterEvent(QDragEnterEvent *e) {
  QMimeData const *md = e->mimeData();
  if (md->hasFormat("int32/point"))
    e->accept();
  else
    e->ignore();
}

void StripView::dragMoveEvent(QDragMoveEvent *e) {
  QMimeData const *md = e->mimeData();
  if (md->hasFormat("int32/point")) {
    QPoint p = e->pos();
    QByteArray ar = md->data("int32/point");
    ASSERT(ar.size()==16);
    qint32 const *d = reinterpret_cast<qint32*>(ar.data());
    QPoint p0(d[0], d[1]);
    QPoint inscene0(d[2], d[3]);
    centerOn(inscene0 + 3*(p0 - p));
    e->accept();
  } else {
    e->ignore();
  }
}
  
void StripView::dragLeaveEvent(QDragLeaveEvent *e) {
  e->accept();
}

void StripView::dropEvent(QDropEvent *e) {
  e->ignore();
}
