// Strip.cpp

#include "Strip.h"
#include "Slide.h"
#include <QPainter>
#include <QSet>
#include "PDebug.h"
#include <QGraphicsSceneMouseEvent>
#include "FilmScene.h"

Strip::Strip(PhotoDB const &db, QGraphicsItem *parent):
  QGraphicsObject(parent), db(db) {
  arr = Arrangement::Vertical;
  scl = TimeScale::None;
  org = Organization::ByDate;
  tilesize = 128;
  rowwidth = 1024;
  expanded = false;
  subheight = -1;
  headerid = 0;
  setPos(1e6, 1e6);
}

Strip::~Strip() {
  setHeaderID(0);
}

void Strip::setHeaderID(quint64 id) {
  if (headerid==id)
    return;
  if (headerid) {
    FilmScene *fs = dynamic_cast<FilmScene *>(scene());
    if (fs)
      fs->dropHeaderFor(headerid, this);
    else
      pDebug() << "Strip not in a scene - disaster imminent";
  }
  if (id) {
    FilmScene *fs = dynamic_cast<FilmScene *>(scene());
    if (fs)
      fs->addHeaderFor(id, this);
    else
      pDebug() << "Strip not in a scene - won't show image";
    headerid = id;
  }
}

int Strip::subHeight() const {
  if (subheight<0)
    subheight = subBoundingRect().height();
  return subheight;
}

QDateTime Strip::startDateTime() const {
  return d0;
}

QDateTime Strip::endDateTime() const {
  return endFor(d0, scl);
}

QDateTime Strip::endFor(QDateTime d0, TimeScale scl) {
  switch (scl) {
  case TimeScale::Eternity:
    return d0.addYears(1000);
  case TimeScale::Decade:
    return d0.addYears(10);
  case TimeScale::Year:
    return d0.addYears(1);
  case TimeScale::Month:
    return d0.addMonths(1);
  case TimeScale::Day:
    return d0.addDays(1);
  case TimeScale::Hour:
    return d0.addSecs(3600);
  case TimeScale::DecaMinute:
    return d0.addSecs(600);
  case TimeScale::None:
    return QDateTime();
  }
  return QDateTime(); // not executed
}

Strip::TimeScale Strip::timeScale() const {
  return scl;
}

int Strip::labelHeight(int) {
  return 16;
}

bool Strip::hasTopLabel() const {
  switch (arr) {
  case Arrangement::Horizontal:
    return false;
  case Arrangement::Vertical:
    return true;
  case Arrangement::Grid:
    switch (org) {
    case Organization::ByDate:
      return scl==TimeScale::Eternity
        || scl==TimeScale::Decade;
    case Organization::ByFolder:
      return pathname=="/";
    }
  }
  return false; // not executed
}

QRectF Strip::labelBoundingRect() const {
  return labelRect;
}

void Strip::recalcLabelRect() {
  prepareGeometryChange();
  switch (arr) {
  case Arrangement::Horizontal:
    if (expanded)
      labelRect = QRectF(0, 0, labelHeight(tilesize), tilesize);
    else
      labelRect = QRectF(0, 0, tilesize, tilesize);
    break;
  case Arrangement::Vertical:
    if (expanded)
      labelRect = QRectF(0, 0, tilesize, labelHeight(tilesize));
    else
      labelRect = QRectF(0, 0, tilesize, tilesize);
    break;
  case Arrangement::Grid:
    if (expanded) {
      if (hasTopLabel()) 
	labelRect = QRectF(0, 0, rowwidth, labelHeight(tilesize));
      else 
	labelRect = QRectF(0, 0, labelHeight(tilesize), subHeight());
    } else {
      labelRect = QRectF(0, 0, tilesize, tilesize);
    }
    break;
  }
  update();
}

QRectF Strip::boundingRect() const {
  QRectF r0 = labelBoundingRect();
  if (!isExpanded())
    return r0;
  
  switch (arr) {
  case Arrangement::Horizontal:
    r0.setWidth(2*r0.width());
    break;
  case Arrangement::Vertical:
    r0.setHeight(2*r0.height());
    break;
  case Arrangement::Grid:
    if (hasTopLabel())
      r0.setHeight((r0 | subBoundingRect()).height());
    else
      r0.setWidth(5*r0.width());
    break;
  }
  return r0;
}

QRectF Strip::subBoundingRect() const {
  return QRectF();
}

QRectF Strip::netBoundingRect() const {
  QRectF r = labelBoundingRect();
  if (expanded)
    r |= subBoundingRect();
  return r;
}

void Strip::paintCollapsedHeaderBox(QPainter *painter, QRectF r, QColor bg) {
  painter->setPen(QPen(Qt::NoPen));
  painter->setBrush(QBrush(QColor(129, 129, 129)));
  painter->drawRoundedRect(r.adjusted(2, 2, 0, 0), 4, 4);
  painter->setBrush(QBrush(QColor(240, 240, 240)));
  painter->drawRoundedRect(r.adjusted(0, 0, -2, -2), 4, 4);
  painter->setBrush(QBrush(bg));
  painter->drawRoundedRect(r.adjusted(1, 1, -1, -1), 4, 4);
}

void Strip::paintExpandedHeaderBox(QPainter *painter, QRectF r, QColor bg) {
  painter->setPen(QPen(Qt::NoPen));
  QPolygonF poly;
  QRectF r0 = boundingRect();
  bool isroot = false;
  switch (org) {
  case Organization::ByDate:
    isroot = scl==TimeScale::Eternity;
    break;
  case Organization::ByFolder:
    isroot = pathname=="/";
    break;
  }
  int slantw = isroot ? 0 : 12;
  if (r.width() >= r.height()) {
    poly << (r.topLeft() + QPointF(slantw+2, 2));
    poly << (r.topRight() + QPointF(-slantw, 2));
    poly << r.bottomRight();
    if (hasTopLabel() && !isroot) {
      poly << r0.bottomRight();
      poly << (r0.bottomLeft() + QPointF(2, 0));
    }
    poly << (r.bottomLeft() + QPointF(2, 0));
  } else {
    if (arr==Arrangement::Grid) 
      r.adjust(0, 2, 0, -2);
    poly << (r.topLeft() + QPointF(2, slantw+2));
    poly << (r.topRight() + QPointF(0, 2));
    if (!isroot) {
      poly << (r0.topRight() + QPointF(0, 2));
      poly << r0.bottomRight();
    }
    poly << r.bottomRight();
    poly << (r.bottomLeft() + QPointF(2, -slantw));
  }      
  painter->setBrush(QBrush(QColor(129, 129, 129)));
  painter->drawPolygon(poly);
  poly.translate(-2, -2);
  painter->setBrush(QBrush(QColor(255, 255, 255)));
  painter->drawPolygon(poly);
  poly.translate(1, 1);
  painter->setBrush(QBrush(bg));
  painter->drawPolygon(poly);
  if (arr==Arrangement::Grid && hasTopLabel() && !isroot) {
    int y0 = r.bottom() + 5;
    int y1 = r0.bottom();
    QLinearGradient gg(0, y0, 0, y1);
    gg.setColorAt(0, bg);
    gg.setColorAt(1, QColor(128, 128, 128));
    painter->setBrush(QBrush(gg));
    painter->drawRect(QRectF(QPointF(r.left()+1, y0),
                             QPointF(r.right()-1, y1)));
  }
}

void Strip::paintHeaderImage(QPainter *painter, QRectF r) {
  if (headerid==0) {
    QSqlQuery q;
    switch (org) {
    case Organization::ByDate:
      q = db.query("select version"
                   " from filter inner join photos"
                   " on filter.photo=photos.id"
                   " where photos.capturedate>=:a"
                   " and photos.capturedate<:b"
                   " limit 1", d0, endFor(d0, scl));
      break;
    case Organization::ByFolder:
      // hmmm.
      break;
    }
    if (q.next())
      setHeaderID(q.value(0).toULongLong());
    else
      qDebug() << "Could not find header image for " << int(org)
               << d0 << endFor(d0, scl) << int(scl)
               << pathname;
  }

  int ims = tilesize - 20;
  if (!(headerpm.width()==ims || headerpm.height()==ims)) {
    if (headerimg.isNull()) {
      requestImage(headerid);
      return;
    } else {
      headerpm = QPixmap::fromImage(headerimg.scaledToFitIn(PSize(ims, ims))
                                    .toQImage());
      headerimg = Image16();
    }
  }
  painter->drawPixmap(r.width()/2 - headerpm.width()/2,
                      r.height()/2 - headerpm.height()/2,
                      headerpm);
}

void Strip::paintHeaderText(QPainter *painter, QRectF r) {
  QString lbl;
  switch (org) {
  case Organization::ByDate:
    lbl = labelFor(d0, scl);
    break;
  case Organization::ByFolder:
    lbl = leafname;
    break;
  }

  if (!expanded) {
    painter->setPen(QPen(QColor(0, 0, 0)));
    painter->drawText(r.adjusted(2, 4, 0, 0), Qt::AlignCenter, lbl);
    painter->drawText(r.adjusted(4, 2, 0, 0), Qt::AlignCenter, lbl);
    painter->drawText(r.adjusted(2, 2, 0, 0), Qt::AlignCenter, lbl);
    painter->drawText(r.adjusted(2, 0, 0, -2), Qt::AlignCenter, lbl);
    painter->drawText(r.adjusted(0, 2, 0, -2), Qt::AlignCenter, lbl);
    painter->drawText(r.adjusted(0, 0, -2, -2), Qt::AlignCenter, lbl);
    painter->setPen(QPen(QColor(255,255,255)));
    painter->drawText(r, Qt::AlignCenter, lbl);
  } else if (r.width() >= r.height()) {
    // Horizontal display
    painter->setPen(QPen(QColor(0, 0, 0)));
    painter->drawText(r, Qt::AlignCenter, lbl);
  } else {
    // Vertical display
    painter->rotate(-90);
    int N = (r.height()+3*tilesize-1)/(3*tilesize);
    int dx = r.height()/N;
    painter->setPen(QPen(QColor(0, 0, 0)));
    for (int n=0; n<N; n++)    
      painter->drawText(QRectF(-r.height()+dx*n, 0, dx, r.width()),
                        Qt::AlignCenter, lbl);
  }  
}

void Strip::paint(QPainter *painter,
                  const QStyleOptionGraphicsItem *,
                  QWidget *) {
  QRectF r = labelBoundingRect();
  int bggray = 192;
  switch (org) {
  case Organization::ByDate:
    if (scl==TimeScale::Decade)
      bggray = 255;
    else
      bggray = (int(scl) & 1) ? 236 : 192;
    break;
  case Organization::ByFolder:
    bggray = (pathname.split("/").size() & 1) ? 236 : 192;
    break;
  }
  QColor bg(bggray, bggray, bggray);
  if (expanded) {
    paintExpandedHeaderBox(painter, r, bg);
  } else {
    paintCollapsedHeaderBox(painter, r, bg);
    paintHeaderImage(painter, r);
  }
  paintHeaderText(painter, r);
}

void Strip::rebuildToolTip() {
  switch (org) {
  case Organization::ByDate:
    setToolTip(longLabelFor(d0, scl));
    break;
  case Organization::ByFolder:
    setToolTip(pathname == "/" ? db.fileName() : pathname);
    break;
  }
}

QString Strip::longLabelFor(QDateTime d0, Strip::TimeScale scl) {
  switch (scl) {
  case TimeScale::Eternity:
    return db.fileName();
  case TimeScale::Decade:
    return d0.toString("yyyy") + QString::fromUtf8("–")
      + d0.addYears(9).toString("yyyy");
  case TimeScale::Year:
    return d0.toString(QString::fromUtf8("yyyy"));
  case TimeScale::Month:
    return d0.toString(QString::fromUtf8("MMMM yyyy"));
  case TimeScale::Day:
    return d0.toString(QString::fromUtf8("MMMM d, yyyy"));
  case TimeScale::Hour:
    return d0.toString("MMMM d, yyyy, h ap");
  case TimeScale::DecaMinute:
    return d0.toString("MMMM d, yyyy, h:mm ap");
  case TimeScale::None:
    return "None";
  }
  return "?";
}
  

QString Strip::labelFor(QDateTime d0, Strip::TimeScale scl) {
  switch (scl) {
  case TimeScale::Eternity:
    return "Library";
  case TimeScale::Decade:
    return d0.toString("yyyy") + QString::fromUtf8("–’")
      + d0.addYears(9).toString("yy");
  case TimeScale::Year:
    return d0.toString("yyyy");
  case TimeScale::Month:
    return d0.toString("MMM yyyy");
  case TimeScale::Day:
    return d0.toString(QString::fromUtf8("M/d"));
  case TimeScale::Hour:
    return d0.toString("h ap");
  case TimeScale::DecaMinute:
    return d0.toString("h:mm");
  case TimeScale::None:
    return "None";
  }
  return "?";
}

Slide *Strip::slideByVersion(quint64) {
  return NULL;
}

void Strip::updateImage(quint64 v, Image16 img) {
  Slide *s = slideByVersion(v);
  if (s)
    s->updateImage(img);  
}

void Strip::setTimeRange(QDateTime t0, TimeScale scl1) {
  org = Organization::ByDate;
  scl = scl1;
  d0 = startFor(t0, scl);
  rebuildToolTip();
  rebuildContents();
}

void Strip::setFolder(QString f) {
  org = Organization::ByFolder;
  pathname = f;
  QStringList bits = f.split("/");
  leafname = bits.isEmpty() ? "" : bits.last();
  if (leafname.isEmpty())
    leafname = "Library";
  rebuildToolTip();
  rebuildContents();
}  

QDateTime Strip::startFor(QDateTime t0, TimeScale scl) {
  switch (scl) {
  case TimeScale::Eternity:
    return QDateTime(QDate(1500, 1, 1), QTime(0, 0, 0));
  case TimeScale::Decade: {
    int yr = t0.date().year();
    while (yr%10 != 1)
      yr--;
    return QDateTime(QDate(yr, 1, 1), QTime(0, 0, 0));
  }
  case TimeScale::Year: {
    int yr = t0.date().year();
    return QDateTime(QDate(yr, 1, 1), QTime(0, 0, 0));
  }
  case TimeScale::Month: {
    int yr = t0.date().year();
    int mo = t0.date().month();
    return QDateTime(QDate(yr, mo, 1), QTime(0, 0, 0));
  }
  case TimeScale::Day:
    return QDateTime(t0.date(), QTime(0, 0, 0));
  case TimeScale::Hour: {
    int h = t0.time().hour();
    return QDateTime(t0.date(), QTime(h, 0, 0));
  }
  case TimeScale::DecaMinute: {
    int h = t0.time().hour();
    int dm = t0.time().minute()/10;
    return QDateTime(t0.date(), QTime(h, 10*dm, 0));
  }
  case TimeScale::None:
    return QDateTime();
  }
  return QDateTime();
}

void Strip::setArrangement(Arrangement arr1) {
  arr = arr1;
  recalcLabelRect();
}

void Strip::setTileSize(int pix) {
  if (pix<10)
    pix = 10;
  tilesize = pix;
  recalcLabelRect();
}

int Strip::subRowWidth(int pix) const {
  return hasTopLabel() ? pix : pix - labelHeight(tilesize);  
}

void Strip::setRowWidth(int pix) {
  rowwidth = pix;
  recalcLabelRect();
}

void Strip::expand() {
  if (expanded)
    return;
  expanded = true;
  if (org==Organization::ByDate)
    db.query("insert into expanded values(:a,:b)", d0, int(scl));
  recalcLabelRect();
  emit resized();
}

void Strip::collapse() {
  if (!expanded)
    return;
  expanded = false;
  if (org==Organization::ByDate)
    db.query("delete from expanded where d0==:a and scl==:b", d0, int(scl));
  recalcLabelRect();
  emit resized();
}

void Strip::expandAll() {
  expand();
}

void Strip::relayout() {
  subheight = -1;
}
	
void Strip::rescan() {
  /* Somehow should avoid closing things that were open */
  rebuildContents();
}

void Strip::clearContents() {
}


void Strip::rebuildContents() {
}

void Strip::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *) {
}

void Strip::mousePressEvent(QGraphicsSceneMouseEvent *e) {
  db.beginAndLock();
  if (e->modifiers() & Qt::ShiftModifier)
    expandAll();
  else if (expanded) 
    collapse();
  else
    expand();
  db.commitAndUnlock();
}

void Strip::mouseReleaseEvent(QGraphicsSceneMouseEvent *) {
}

void Strip::updateHeader(Image16 img) {
  headerimg = img;
  update();
}

void Strip::requestImage(quint64 id) {
  emit needImage(id, PSize(tilesize, tilesize));
}

Strip *Strip::stripByDate(QDateTime d, TimeScale s) {
  if (d==d0 && s==scl)
    return this;
  else
    return NULL;
}

Strip *Strip::stripByFolder(QString path) {
  if (path==pathname)
    return this;
  else
    return NULL;
}

quint64 Strip::versionLeftOf(quint64 vsn) {
  return versionAt(vsn, QPoint(-1, 0));
}

quint64 Strip::versionRightOf(quint64 vsn) {
  return versionAt(vsn, QPoint(1, 0));
}

quint64 Strip::versionAbove(quint64 vsn) {
  return versionAt(vsn, QPoint(0, -1));
}

quint64 Strip::versionBelow(quint64 vsn) {
  return versionAt(vsn, QPoint(0, 1));
}
