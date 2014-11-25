// Filmstrip.cpp

#include "Filmstrip.h"
#include <QPainter>
#include <QSet>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>

#define MAXDIRECT 50

Filmstrip::Filmstrip(PhotoDB const &db, QGraphicsItem *parent):
  QGraphicsObject(parent), db(db) {
  arr = Arrangement::Vertical;
  scl = TimeScale::None;
  tilesize = 128;
  rowwidth = 1024;
  showheader = true;
  expanded = false;
  rebuilding = false;
  setPos(1e6, 1e6);
}

Filmstrip::~Filmstrip() {
}


QDateTime Filmstrip::startDateTime() const {
  return d0;
}

QDateTime Filmstrip::endDateTime() const {
  return endFor(d0, scl);
}

QDateTime Filmstrip::endFor(QDateTime d0, TimeScale scl) {
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

Filmstrip::TimeScale Filmstrip::timeScale() const {
  return scl;
}

void Filmstrip::showHeader() {
  showheader = true;
  relayout();
}

void Filmstrip::hideHeader() {
  showheader = false;
  relayout();
}

int Filmstrip::labelHeight(int) {
  return 16;
}

bool Filmstrip::hasTopLabel() const {
  return scl==TimeScale::Eternity
    || scl==TimeScale::Decade
    || scl==TimeScale::Year;
}

QRectF Filmstrip::boundingRect() const {
  if (!showheader)
    return QRectF(0, 0, 1, 1);
  
  switch (arr) {
  case Arrangement::Horizontal:
    if (expanded)
      return QRectF(0, 0, tilesize, tilesize);
    else
      return QRectF(0, 0, labelHeight(tilesize), tilesize);
  case Arrangement::Vertical:
    if (expanded)
      return QRectF(0, 0, tilesize, tilesize);
    else
      return QRectF(0, 0, tilesize, labelHeight(tilesize));
  case Arrangement::Grid:
    if (expanded) {
      if (hasTopLabel()) 
	return QRectF(0, 0, rowwidth, labelHeight(tilesize));
      else 
	return QRectF(0, 0, labelHeight(tilesize), subBoundingRect().height());
    } else {
      return QRectF(0, 0, tilesize, tilesize);
    }
  }
  return QRectF(0, 0, 1, 1); // not executed
}

QRectF Filmstrip::subBoundingRect() const {
  if (!slides.isEmpty()) {
    int lw = (scl==TimeScale::Eternity
	      || scl==TimeScale::Decade
	      || scl==TimeScale::Year)
      ? 0 : labelHeight(tilesize);
    int lh = lw ? 0 : labelHeight(tilesize);
    int perrow = (rowwidth - lw) / tilesize;
    int rows = slides.size()/perrow;
    if (slides.size()%perrow>0)
      rows++;
//    qDebug() << "subBoundingRect" << d0 << int(scl) << ":"
//	     << lw << perrow << rows << slides.size()
//	     << ":"
//	     << QRectF(QPointF(lw, lh), QSizeF(tilesize*perrow, tilesize*rows));
    return QRectF(QPointF(lw, lh), QSizeF(tilesize*perrow, tilesize*rows));
  } else {
    QRectF r;
    for (auto f: subStrips)
      r |= f->mapRectToParent(f->netBoundingRect());
    return r;
  }
}

QRectF Filmstrip::netBoundingRect() const {
  QRectF r = boundingRect();
  if (expanded)
    r |= subBoundingRect();
  return r;
}

void Filmstrip::paint(QPainter *painter,
		      const QStyleOptionGraphicsItem *,
		      QWidget *) {
  if (!showheader)
    return;

  QRectF r = boundingRect();

  if (expanded) {
    painter->setPen(QPen(Qt::NoPen));
    painter->setBrush(QBrush(QColor(0, 0, 0)));
    QPolygonF poly;
    int slantw = 16; // tilesize/2;
    if (r.width() >= r.height()) {
      poly << (r.topLeft() + QPointF(slantw+2, 2));
      poly << (r.topRight() + QPointF(-slantw, 2));
      poly << r.bottomRight();
      poly << (r.bottomLeft() + QPointF(2, 0));
    } else {
      poly << (r.topLeft() + QPointF(2, slantw+2));
      poly << (r.topRight() + QPointF(0, 2));
      poly << r.bottomRight();
      poly << (r.bottomLeft() + QPointF(2, -slantw));
    }      
    painter->drawPolygon(poly);
    poly.translate(-2, -2);
    painter->setBrush(QBrush(QColor(255, 255, 255)));
    painter->drawPolygon(poly);
    poly.translate(1, 1);
    painter->setBrush(QBrush(QColor(127, 127, 127)));
    painter->drawPolygon(poly); 
  } else {
    painter->setPen(QPen(Qt::NoPen));
    painter->setBrush(QBrush(QColor(0, 0, 0)));
    painter->drawRoundedRect(r.adjusted(2, 2, 0, 0), 4, 4);
    painter->setBrush(QBrush(QColor(255, 255, 255)));
    painter->drawRoundedRect(r.adjusted(0, 0, -2, -2), 4, 4);
    painter->setBrush(QBrush(QColor(127, 127, 127)));
    painter->drawRoundedRect(r.adjusted(1, 1, -1, -1), 4, 4);
  }

  painter->setPen(QPen(QColor(255, 255, 255)));
  QString lbl = labelFor(d0, scl);
  
  if (r.width() >= r.height()) {
    // Horizontal display
    painter->drawText(r, Qt::AlignCenter, lbl);
  } else {
    // Vertical display
    painter->rotate(-90);
    painter->drawText(QRectF(-r.height(), 0, r.height(), r.width()),
		      Qt::AlignCenter, lbl);
  }  
}

QString Filmstrip::labelFor(QDateTime d0, Filmstrip::TimeScale scl) {
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
    return d0.toString(QString::fromUtf8("MM/dd/’yy"));
  case TimeScale::Hour:
    return d0.toString("h ap");
  case TimeScale::DecaMinute:
    return d0.toString("h:mm") + QString::fromUtf8("–")
      + d0.addSecs(9*60).toString(":mm");
  case TimeScale::None:
    return "None";
  }
  return "?";
}


Filmstrip *Filmstrip::descendentByDate(QDateTime d) {
  if (d<startDateTime() || d>=endDateTime())
    return 0;

  for (auto s: subStrips) {
    Filmstrip *f = s->descendentByDate(d);
    if (f)
      return f;
  }

  return this;
}


Slide *Filmstrip::slideByVersion(quint64 vsn) {
  if (slides.contains(vsn))
    return slides[vsn];

  for (auto f: subStrips) {
    Slide *s = f->slideByVersion(vsn);
    if (s)
      return s;
  }

  return 0;
}

void Filmstrip::updateImage(quint64 v, QSize, QImage img) {
  Slide *s = slideByVersion(v);
  if (s)
    s->updateImage(img);  
}

void Filmstrip::setTimeRange(QDateTime t0, TimeScale scl1) {
  scl = scl1;
  d0 = startFor(t0, scl);
  clearContents();
  rebuildContents();
}

QDateTime Filmstrip::startFor(QDateTime t0, TimeScale scl) {
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

void Filmstrip::setArrangement(Arrangement arr1) {
  prepareGeometryChange();
  arr = arr1;
  for (auto f: subStrips)
    f->setArrangement(arr1);
  relayout();
}

void Filmstrip::setTileSize(int pix) {
  prepareGeometryChange();
  tilesize = pix;
  for (auto f: subStrips)
    f->setTileSize(pix);
  for (auto s: slides)
    s->setTileSize(pix);
  relayout();
}

int Filmstrip::subRowWidth(int pix) const {
  return (scl==TimeScale::Eternity
	  || scl==TimeScale::Decade
	  || scl==TimeScale::Year)
    ? pix : pix - labelHeight(tilesize);  
}

void Filmstrip::setRowWidth(int pix) {
  prepareGeometryChange();
  rowwidth = pix;
  int subwidth = subRowWidth(pix);
  for (auto f: subStrips)
    f->setRowWidth(subwidth);
  relayout();
}

void Filmstrip::expand() {
  prepareGeometryChange();
  expanded = true;
  for (auto c: orderedChildren)
    c->show();
  if (needsRebuild)
    rebuildContents();
  else
    relayout();
  update();
}

void Filmstrip::collapse() {
  prepareGeometryChange();
  expanded = false;
  for (auto c: orderedChildren)
    c->hide();
  update();
  emit resized();
}

void Filmstrip::expandAll() {
  expand();
  for (auto f: subStrips)
    f->expandAll();
}

void Filmstrip::collapseAll() {
  collapse();
  for (auto f: subStrips)
    f->collapseAll();
}

void Filmstrip::relayout() {
  if (!expanded)
    return;
  if (rebuilding)
    return;
  //  qDebug() << "relayout " << d0 << int(scl);

  prepareGeometryChange();
    
  switch (arr) {
  case Arrangement::Horizontal: {
    int x = boundingRect().right();
    for (auto c: orderedChildren) {
      c->setPos(x, 0);
      Filmstrip *s = dynamic_cast<Filmstrip *>(c);
      if (s)
	x += s->netBoundingRect().width();
      else 
	x += c->boundingRect().width();
    }
  } break;
  case Arrangement::Vertical: {
    int y = boundingRect().bottom();
    for (auto c: orderedChildren) {
      c->setPos(0, y);
      Filmstrip *s = dynamic_cast<Filmstrip *>(c);
      if (s)
	y += s->netBoundingRect().height();
      else 
	y += c->boundingRect().height();
    }
  } break;
  case Arrangement::Grid: {
    bool hastoplabel = hasTopLabel();
    int x0 = hastoplabel ? 0 : labelHeight(tilesize);
    int y0 = hastoplabel ? labelHeight(tilesize) : 0;
    int edy = subStrips.isEmpty() ? 0 : 2;
    int dy = 0;
    int x = x0;
    int y = y0;
    bool atstart = true;
    for (auto c: orderedChildren) {
      Filmstrip *s = dynamic_cast<Filmstrip *>(c);
      bool ex = s ? s->isExpanded() : false;
      QRectF r1 = s ? s->netBoundingRect() : c->boundingRect();
      if ((ex || x+r1.width()>rowwidth) && !atstart) {
	y += dy + edy;
	x = x0;
	dy = 0;
	atstart = true;
      }
      c->setPos(x, y);
      if (r1.height()>dy)
	dy = r1.height();
      if (ex) {
	x = x0;
	y += dy + edy;
	dy = 0;
	atstart = true;
      } else {
	x += r1.width();	
	atstart = false;
      }
    }
  } break;
  }
  emit resized();
}
	
void Filmstrip::rescan() {
  rebuildContents();
}

void Filmstrip::clearContents() {
  for (auto c: orderedChildren) {
    // removeChild(c);
    delete c;
  }
  orderedChildren.clear();
  subStrips.clear();
  slides.clear();
}  

int Filmstrip::countInRange(QDateTime begin, QDateTime end) const {
  QSqlQuery q(*db);
  q.prepare("select count(*)"
	    " from versions inner join photos"
	    " on versions.photo=photos.id"
	    " where photos.capturedate>=:a and photos.capturedate<:b");
  q.bindValue(":a", begin);
  q.bindValue(":b", end);
  //  qDebug() << "countinrange" << begin << end;
  if (!q.exec() || !q.next())
    throw q;
  //  qDebug() << "  counted versions: " << q.value(0).toInt();
  return q.value(0).toInt();
}

bool Filmstrip::anyInRange(QDateTime begin, QDateTime end) const {
  QSqlQuery q(*db);
  q.prepare("select count(*)"
	    " from photos"
	    " where capturedate>=:a and capturedate<:b");
  q.bindValue(":a", begin);
  q.bindValue(":b", end);
  //  qDebug() << "anyinrange" << begin << end;
  if (!q.exec() || !q.next())
    throw q;
  //  qDebug() << "  counted photos: " << q.value(0).toInt();
  return q.value(0).toInt() > 0;
}

QDateTime Filmstrip::firstDateIn(QDateTime t0, QDateTime t1) const {
  QSqlQuery q(*db);
  q.prepare("select capturedate from photos"
	    " where capturedate>=:a and photos.capturedate<:b"
	    " order by capturedate limit 1");
  q.bindValue(":a", t0);
  q.bindValue(":b", t1);
  //  qDebug() << "firstdatein" << t0 << t1;
  if (!q.exec())
    throw q;
  if (!q.next())
    return QDateTime();
  return q.value(0).toDateTime();
}

QDateTime Filmstrip::lastDateIn(QDateTime t0, QDateTime t1) const {
  QSqlQuery q(*db);
  q.prepare("select capturedate from photos"
	    " where capturedate>=:a and photos.capturedate<:b"
	    " order by capturedate desc"
	    " limit 1");
  q.bindValue(":a", t0);
  q.bindValue(":b", t1);
  //  qDebug() << "lastdatein" << t0 << t1;
  if (!q.exec())
    throw q;
  if (!q.next())
    return QDateTime();
  return q.value(0).toDateTime();
}

QList<quint64> Filmstrip::versionsInRange(QDateTime begin,
					  QDateTime end) const {
  QSqlQuery q(*db);
  q.prepare("select versions.id"
	    " from versions inner join photos"
	    " on versions.photo=photos.id"
	    " where photos.capturedate>=:a and photos.capturedate<:b"
	    " order by photos.capturedate");
  q.bindValue(":a", begin);
  q.bindValue(":b", end);
  //  qDebug() << "versionsinrange" << begin << end;
  if (!q.exec())
    throw q;
  QList<quint64> vv;
  while (q.next())
    vv << q.value(0).toULongLong();
  return vv;
}

void Filmstrip::rebuildContents() {
  if (expanded) {
    rebuilding = true;
    int n = countInRange(startDateTime(), endDateTime());
    
    orderedChildren.clear(); // will rebuild
    
    if ((n>=MAXDIRECT && scl!=TimeScale::DecaMinute)
	|| !subStrips.isEmpty()) {
      rebuildContentsWithSubstrips();
      showheader = true; // subStrips.size() > 1;
    } else {
      rebuildContentsDirectly();
      showheader = true;
    }
    rebuilding = false;
    relayout();
  } else {
    needsRebuild = true;
  }
}

void Filmstrip::rebuildContentsWithSubstrips() {
  for (auto s: slides) 
    delete s;
  slides.clear();
    
  TimeScale subs =
    scl==TimeScale::Eternity ? TimeScale::Decade
    : scl==TimeScale::Decade ? TimeScale::Year
    : scl==TimeScale::Year ? TimeScale::Month
    : scl==TimeScale::Month ? TimeScale::Day
    : scl==TimeScale::Day ? TimeScale::Hour
    : TimeScale::DecaMinute;
  QDateTime end = endDateTime();
  QDateTime start = firstDateIn(startDateTime(), end);
  if (start.isNull()) {
    clearContents();
    return;
  }
  end = lastDateIn(start, end);
  QDateTime t1;
  for (QDateTime t=startFor(start, subs); t<=end; t=t1) {
    t1 = endFor(t, subs);
    Q_ASSERT(t1>t);
    if (anyInRange(t, t1)) {
      // build or rebuild substrip
      if (!subStrips.contains(t)) {
	subStrips[t] = new Filmstrip(db, this);
	connect(subStrips[t], SIGNAL(needImage(quint64, QSize)),
		this, SIGNAL(needImage(quint64, QSize)));
	connect(subStrips[t], SIGNAL(pressed(quint64)),
		this, SIGNAL(pressed(quint64)));
	connect(subStrips[t], SIGNAL(clicked(quint64)),
		this, SIGNAL(clicked(quint64)));
	connect(subStrips[t], SIGNAL(doubleClicked(quint64)),
		this, SIGNAL(doubleClicked(quint64)));
	subStrips[t]->setArrangement(arr);
	subStrips[t]->setTileSize(tilesize);
	subStrips[t]->setRowWidth(subRowWidth(rowwidth));
	connect(subStrips[t], SIGNAL(resized()),
		this, SLOT(relayout()), Qt::QueuedConnection);
	// This could cause excessive calls to relayout--I need to check
      }
      subStrips[t]->setTimeRange(t, subs);
      orderedChildren << subStrips[t];
      if (expanded)
	subStrips[t]->show();
      else
	subStrips[t]->hide();
    } else {
      if (subStrips.contains(t)) {
	delete subStrips[t];
	subStrips.remove(t);
      }
    }
    t = t1;
  }
}

void Filmstrip::requestImage(quint64 id) {
  emit needImage(id, QSize(tilesize, tilesize));
}

void Filmstrip::slidePressed(quint64 id) {
  emit pressed(id);
}

void Filmstrip::slideClicked(quint64 id) {
  emit clicked(id);
}

void Filmstrip::slideDoubleClicked(quint64 id) {
  emit doubleClicked(id);
}

void Filmstrip::rebuildContentsDirectly() {
  for (auto s: subStrips)
    delete s;
  subStrips.clear();
  
  QSet<quint64> keep;
  QList<quint64> vv = versionsInRange(startDateTime(), endDateTime());
  for (auto id: vv) {
    if (!slides.contains(id)) {
      slides[id] = new Slide(id, this);
      slides[id]->setTileSize(tilesize);
      if (expanded)
	slides[id]->show();
      else
	slides[id]->hide();
    }
    orderedChildren << slides[id];
    keep << id;
  }

  for (auto id: slides.keys()) {
    if (!keep.contains(id)) {
      // removeItem(slides[id]);
      delete slides[id];
      slides.remove(id);
    }
  }
}

void Filmstrip::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *) {
}

void Filmstrip::mousePressEvent(QGraphicsSceneMouseEvent *e) {
  if (e->modifiers() & Qt::ControlModifier)
    expandAll();
  else if (e->modifiers() & Qt::ShiftModifier)
    collapseAll();
  else if (expanded)
    collapse();
  else
    expand();
}

void Filmstrip::mouseReleaseEvent(QGraphicsSceneMouseEvent *) {
}
