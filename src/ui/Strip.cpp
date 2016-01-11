// Strip.cpp

#include "Strip.h"
#include "Slide.h"
#include <QPainter>
#include <QSet>
#include "PDebug.h"
#include <QGraphicsSceneMouseEvent>
#include "StripScene.h"
#include "Selection.h"

Strip::Strip(PhotoDB *db, QGraphicsItem *parent):
  QGraphicsObject(parent), db(db) {
  hasheader = true;
  arr = Arrangement::Vertical;
  scl = TimeScale::None;
  org = Organization::ByDate;
  tilesize = 128;
  rowwidth = 1024;
  expanded = false;
  headerid = 0;
  setPos(1e6, 1e6);
}

Strip::~Strip() {
  setHeaderID(0);
}

void Strip::makeHeaderless() {
  hasheader = false;
  recalcLabelRect();
  update();
}

void Strip::setHeaderID(quint64 id) {
  if (headerid==id)
    return;
  if (headerid) {
    StripScene *fs = dynamic_cast<StripScene *>(scene());
    if (fs)
      fs->dropHeaderFor(headerid, this);
    else
      qDebug() << "Strip not in a scene - disaster imminent";
  }
  if (id) {
    StripScene *fs = dynamic_cast<StripScene *>(scene());
    if (fs)
      fs->addHeaderFor(id, this);
    else
      qDebug() << "Strip not in a scene - won't show image";
    headerid = id;
  }
}

int Strip::subHeight() const {
  return subBoundingRect().height();
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
  QFont f;
  QFontMetrics fm(f);
  return fm.boundingRect("0").height();
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
  int lh = hasheader ? labelHeight(tilesize) : 1;
  switch (arr) {
  case Arrangement::Horizontal:
    if (expanded)
      labelRect = QRectF(0, 0, lh, tilesize);
    else
      labelRect = QRectF(0, 0, tilesize, tilesize);
    break;
  case Arrangement::Vertical:
    if (expanded)
      labelRect = QRectF(0, 0, tilesize, lh);
    else
      labelRect = QRectF(0, 0, tilesize, tilesize);
    break;
  case Arrangement::Grid:
    if (expanded) {
      if (hasTopLabel()) 
	labelRect = QRectF(0, 0, rowwidth, lh);
      else 
	labelRect = QRectF(0, 0, lh, subHeight());
    } else {
      labelRect = QRectF(0, 0, tilesize, tilesize);
    }
    break;
  }
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
      r0.setHeight(2*r0.height());
    else
      r0.setWidth(2*r0.width());
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

void Strip::toggleSelection() {
  int n=0;
  int N=0;
  switch (org) {
  case Organization::ByDate:
    N = db->countInDateRange(d0, endFor(d0, scl));
    if (N) {
      n = Selection(db).countInDateRange(d0, endFor(d0, scl));
      if (n==N)
        Selection(db).dropDateRange(d0, scl);
      else
        Selection(db).addDateRange(d0, scl);
    }
    break;
  case Organization::ByFolder:
    N = db->countInTree(pathname);
    if (N) {
      n = Selection(db).countInTree(pathname);
      if (n==N)
        Selection(db).dropInTree(pathname);
      else
        Selection(db).addInTree(pathname);
    }
  }
  update();
}
  

void Strip::paintCollapsedHeaderBox(QPainter *painter, QRectF r, QColor bg) {
  int n=0;
  int N=0;
  switch (org) {
  case Organization::ByDate:
    N = db->countInDateRange(d0, endFor(d0, scl));
    if (N)
      n = Selection(db).countInDateRange(d0, endFor(d0, scl));
    break;
  case Organization::ByFolder:
    N = db->countInTree(pathname);
    if (N)
      n = Selection(db).countInTree(pathname);
    break;
  }
  painter->setPen(QPen(Qt::NoPen));
  painter->setBrush(QBrush(QColor(129, 129, 129)));
  painter->drawRoundedRect(r.adjusted(2, 2, 0, 0), 4, 4);
  painter->setBrush(QBrush(QColor(240, 240, 240)));
  painter->drawRoundedRect(r.adjusted(0, 0, -2, -2), 4, 4);
  int dx = 1;
  if (n>0) {
    if (n==N) 
      painter->setBrush(QColor("#ff8800"));
    else
      painter->setBrush(dashPattern(bg));
    painter->drawRoundedRect(r.adjusted(1, 1, -1, -1), 4, 4);
    dx = 3;
  }
  painter->setBrush(QBrush(bg));
  painter->drawRoundedRect(r.adjusted(dx, dx, -dx, -dx), 4, 4);
}

QPixmap const &Strip::dashPattern(QColor bg1) {
  static QPixmap pm;
  static QColor bg;
  if (pm.isNull() || bg1!=bg) {
    bg = bg1;
    QRgb rgb = bg.rgba();
    constexpr int L = 6;
    QImage img(L*2, L*2, QImage::Format_RGB32);
    for (int y=0; y<2*L; y++) {
      quint32 *ptr = reinterpret_cast<quint32*>(img.scanLine(y));
      quint32 *p = (y>=L) ? ptr : ptr + L;
      for (int x=0; x<L; x++)
        *p++ = 0xffff8800;
      p = (y>=L) ? ptr + L : ptr;
      for (int x=0; x<L; x++)
        *p++ = rgb;
      
      
    }
    pm = QPixmap::fromImage(img);
  }
  return pm;
}
  

void Strip::paintExpandedHeaderBox(QPainter *painter, QRectF/* r*/, QColor bg) {
  painter->setPen(QPen(Qt::NoPen));
  QPolygonF poly;
  QRectF r = boundingRect().adjusted(1, 1, -1, -1);
  bool isroot = false;
  switch (org) {
  case Organization::ByDate:
    isroot = scl==TimeScale::Eternity;
    break;
  case Organization::ByFolder:
    isroot = pathname=="/";
    break;
  }
  if (isroot) {
    painter->setPen(QPen(QColor(129, 129, 129), 2));
    poly << QPointF(r.bottomLeft());
    poly << QPointF(r.bottomRight());
    poly << QPointF(r.topRight());
    painter->drawPolyline(poly);
    poly.clear();
    painter->setPen(QPen(QColor(255, 255, 255), 2));
    poly << QPointF(r.bottomLeft());
    poly << QPointF(r.topLeft());
    poly << QPointF(r.topRight());
    painter->drawPolyline(poly);
    painter->setPen(QPen(Qt::NoPen));
    painter->setBrush(QBrush(bg));
    painter->drawRect(r);
  } else {    
    int slantw = labelHeight(tilesize)-4;
    if (r.width() >= r.height()) {
      painter->setPen(QPen(QColor(129, 129, 129), 2));
      poly << QPointF(r.bottomRight()) + QPointF(0, -1);
      poly << QPointF(r.topRight()) + QPointF(0, slantw);
      poly << QPointF(r.topRight()) + QPointF(-slantw, 0);
      painter->drawPolyline(poly);
      poly.clear();
      painter->setPen(QPen(QColor(255, 255, 255), 2));
      poly << QPointF(r.bottomLeft());
      poly << QPointF(r.topLeft()) + QPointF(0, slantw-1);
      poly << QPointF(r.topLeft()) + QPointF(slantw-1, 0);
      poly << QPointF(r.topRight()) + QPointF(-slantw-1, 0);
      painter->drawPolyline(poly);
      poly.clear();
      painter->setPen(QPen(Qt::NoPen));
      painter->setBrush(QBrush(bg));
      poly << QPointF(r.bottomLeft());
      poly << QPointF(r.topLeft()) + QPointF(0, slantw);
      poly << QPointF(r.topLeft()) + QPointF(slantw, 0);
      poly << QPointF(r.topRight()) + QPointF(-slantw, 0);
      poly << QPointF(r.topRight()) + QPointF(0, slantw);
      poly << QPointF(r.bottomRight());
      painter->drawPolygon(poly);
    } else {
      painter->setPen(QPen(QColor(129, 129, 129), 2));
      poly << QPointF(r.bottomRight()) + QPointF(-1, 0);
      poly << QPointF(r.bottomLeft()) + QPointF(slantw-1, 0);
      poly << QPointF(r.bottomLeft()) + QPointF(0, -slantw+1);
      painter->drawPolyline(poly);
      poly.clear();
      painter->setPen(QPen(QColor(255, 255, 255), 2));
      poly << QPointF(r.bottomLeft()) + QPointF(0, -slantw);
      poly << QPointF(r.topLeft()) + QPointF(0, slantw-1);
      poly << QPointF(r.topLeft()) + QPointF(slantw-1, 0);
      poly << QPointF(r.topRight());
      painter->drawPolyline(poly);
      poly.clear();
      painter->setPen(QPen(Qt::NoPen));
      painter->setBrush(QBrush(bg));
      poly << QPointF(r.bottomLeft()) + QPointF(0, -slantw);
      poly << QPointF(r.topLeft()) + QPointF(0, slantw);
      poly << QPointF(r.topLeft()) + QPointF(slantw, 0);
      poly << QPointF(r.topRight());
      poly << QPointF(r.bottomRight());
      poly << QPointF(r.bottomLeft()) + QPointF(slantw, 0);
      painter->drawPolygon(poly);
    }
  }
}

void Strip::paintHeaderImage(QPainter *painter, QRectF r) {
  if (headerid==0) {
    switch (org) {
    case Organization::ByDate: {
      QSqlQuery q = db->query("select version"
                             " from filter inner join photos"
                             " on filter.photo=photos.id"
                             " where photos.capturedate>=:a"
                             " and photos.capturedate<:b"
                             " limit 1", d0, endFor(d0, scl));
      if (q.next())
        setHeaderID(q.value(0).toULongLong());
      else
        qDebug() << "Could not find header image for " << int(org)
                 << d0 << endFor(d0, scl) << int(scl)
                 << pathname;
    } break;
    case Organization::ByFolder:
      setHeaderID(db->firstVersionInTree(pathname));
      break;
    }
  }

  int ims = tilesize - 20;
  if (!(headerpm.width()==ims || headerpm.height()==ims)) {
    if (headerimg.isNull()) {
      requestImage(headerid);
      return;
    } else {
      headerpm = QPixmap::fromImage(headerimg
				    .scaledToFitSnuglyIn(PSize(ims, ims))
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
  int n = 0;
  switch (org) {
  case Organization::ByDate:
    lbl = labelFor(d0, scl);
    if (!expanded)
      n = db->countInDateRange(d0, endFor(d0, scl));
    break;
  case Organization::ByFolder:
    lbl = leafname;
    if (!expanded)
      n = db->countInTree(pathname);
    break;
  }

  if (!expanded) {
    lbl += QString("\n(%1)").arg(n);
    r = r.adjusted(2, 2, -2, -2);
    painter->setPen(QPen(QColor(0, 0, 0)));
    painter->drawText(r.translated(1, 2),
                      Qt::AlignCenter | Qt::TextWordWrap, lbl);
    painter->drawText(r.translated(2, 1),
                      Qt::AlignCenter | Qt::TextWordWrap, lbl);
    painter->drawText(r.translated(1, 1),
                      Qt::AlignCenter | Qt::TextWordWrap, lbl);
    painter->drawText(r.translated(-1, 0),
                      Qt::AlignCenter | Qt::TextWordWrap, lbl);
    painter->drawText(r.translated(0, -1),
                      Qt::AlignCenter | Qt::TextWordWrap, lbl);
    painter->drawText(r.translated(-1, -1),
                      Qt::AlignCenter | Qt::TextWordWrap, lbl);
    painter->setPen(QPen(QColor(255,255,255)));
    painter->drawText(r, Qt::AlignCenter | Qt::TextWordWrap, lbl);
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
  //  pDebug() << "Paint" << d0 << int(scl) << netBoundingRect() << labelBoundingRect();
  QRectF r = labelBoundingRect();
  int bggray = 192;
  switch (org) {
  case Organization::ByDate:
    bggray = 160 + 16*(int(scl)-int(TimeScale::Decade));
    break;
  case Organization::ByFolder:
    bggray = ((pathname.split("/").size() + (leafname==""?1:0)) & 1)
      ? 192 : 160;
    break;
  }
  if (hasheader) {
    QColor bg(bggray, bggray, bggray);
    if (expanded) {
      paintExpandedHeaderBox(painter, r, bg);
    } else {
      paintCollapsedHeaderBox(painter, r, bg);
      paintHeaderImage(painter, r);
    }
    paintHeaderText(painter, r);
  }
}

void Strip::rebuildToolTip() {
  switch (org) {
  case Organization::ByDate:
    setToolTip(longLabelFor(d0, scl));
    break;
  case Organization::ByFolder:
    setToolTip(pathname == "/" ? db->name() : pathname);
    break;
  }
}

QString Strip::longLabelFor(QDateTime d0, Strip::TimeScale scl) {
  switch (scl) {
  case TimeScale::Eternity:
    return db->name();
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

void Strip::updateImage(quint64 v, Image16 img, bool chgd) {
  Slide *s = slideByVersion(v);
  if (s)
    s->updateImage(img, chgd);  
}

void Strip::setTimeRange(QDateTime t0, TimeScale scl1) {
  org = Organization::ByDate;
  scl = scl1;
  d0 = startFor(t0, scl);
  rebuildToolTip();
  rebuildContents();
}

void Strip::setDisplayName(QString s) {
  leafname = s;
  update();
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
  update();
}

void Strip::setTileSize(int pix) {
  if (pix<10)
    pix = 10;
  tilesize = pix;
  recalcLabelRect();
  update();
}

int Strip::subRowWidth(int pix) const {
  return (hasTopLabel() || !hasheader) ? pix : pix - labelHeight(tilesize);  
}

void Strip::setRowWidth(int pix) {
  rowwidth = pix;
  recalcLabelRect();
  update();
}

void Strip::expand() {
  if (expanded)
    return;
  expanded = true;
  prepareGeometryChange();
  Untransaction t(db);
  switch (org) {
  case Organization::ByDate:
    db->query("insert into expanded values(:a,:b)", d0, int(scl));
    break;
  case Organization::ByFolder:
    db->query("insert into expandedfolders values(:a)", pathname);
    break;
  }
}

void Strip::collapse() {
  if (!expanded)
    return;
  prepareGeometryChange();
  expanded = false;
  Untransaction t(db);
  switch (org) {
  case Organization::ByDate:
    db->query("delete from expanded where d0==:a and scl==:b", d0, int(scl));
    break;
  case Organization::ByFolder:
    db->query("delete from expandedfolders where path==:a", pathname);
    break;
  }
}

void Strip::expandAll() {
  expand();
}

void Strip::relayout() {
  prepareGeometryChange();
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
  if (!labelRect.contains(e->pos()))
    return;
  
  if (e->modifiers() & Qt::ControlModifier)
    toggleSelection();
  else if (e->modifiers() & Qt::ShiftModifier) 
    expandAll();
  else if (expanded) 
    collapse();
  else
    expand();
}

void Strip::mouseReleaseEvent(QGraphicsSceneMouseEvent *) {
}

void Strip::updateHeader(Image16 img, bool chgd) {
  if (chgd || img.size()>headerimg.size())
    headerimg = img;
  update();
}

void Strip::updateHeaderRotation(int dphi) {
  dphi = dphi & 3;
  switch (dphi) {
  case 0:
    break;
  case 1:
    headerimg.rotate90CCW();
    break;
  case 2:
    headerimg.rotate180();
    break;
  case 3:
    headerimg.rotate90CW();
    break;
  }
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

quint64 Strip::versionBefore(quint64 vsn) {
  switch (arrangement()) {
  case Arrangement::Horizontal:
  case Arrangement::Grid:
    return versionLeftOf(vsn);
  case Arrangement::Vertical:
    return versionAbove(vsn);
  }
  return 0; // not executed
}

quint64 Strip::versionAfter(quint64 vsn) {
  switch (arrangement()) {
  case Arrangement::Horizontal:
  case Arrangement::Grid:
    return versionRightOf(vsn);
  case Arrangement::Vertical:
    return versionBelow(vsn);
  }
  return 0; // not executed
}
