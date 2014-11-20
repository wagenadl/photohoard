// Filmstrip.cpp

#include "Filmstrip.h"
#include <QPainter>
#include <QSet>
#include <QDebug>

#define MAXDIRECT 50

Filmstrip::Filmstrip(PhotoDB const &db, QGraphicsItem *parent):
  QGraphicsObject(parent), db(db) {
  arr = Arrangement::Vertical;
  scl = TimeScale::None;
  tilesize = 128;
  rowwidth = 1024;
  showheader = true;
  expanded = true;
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
  case TimeScale::None:
    return d0;
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
  }
  return d0; // not executed
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

QRectF Filmstrip::boundingRect() const {
  if (!showheader)
    return QRectF(0, 0, 1, 1);
  
  switch (arr) {
  case Arrangement::Horizontal:
    return QRectF(0, 0, tilesize/6, tilesize);
  case Arrangement::Vertical:
    return QRectF(0, 0, tilesize, tilesize/6);
  case Arrangement::Grid:
    if (scl==TimeScale::Decade || scl==TimeScale::Year)
      return QRectF(0, 0, rowwidth, tilesize/6);
    else
      return QRectF(0, 0, tilesize/6, tilesize);
  }
  return QRectF(0, 0, 1, 1); // not executed
}

QRectF Filmstrip::netBoundingRect() const {
  QRectF r = boundingRect();
  if (expanded) {
    for (auto s: slides)
      r |= s->mapRectToParent(s->boundingRect());
    for (auto f: subStrips)
      r |= f->mapRectToParent(f->netBoundingRect());
  }
  return r;
}

void Filmstrip::paint(QPainter *painter,
		      const QStyleOptionGraphicsItem *,
		      QWidget *) {
  if (!showheader)
    return;

  QRectF r = boundingRect();

  painter->setPen(QPen(Qt::NoPen));
  painter->setBrush(QBrush(QColor(0, 0, 0)));
  painter->drawRoundedRect(r.adjusted(2, 2, 0, 0), 4, 4);
  painter->setBrush(QBrush(QColor(255, 255, 255)));
  painter->drawRoundedRect(r.adjusted(0, 0, -2, -2), 4, 4);
  painter->setBrush(QBrush(QColor(127, 127, 127)));
  painter->drawRoundedRect(r.adjusted(1, 1, -1, -1), 4, 4);

  painter->setPen(QPen(QColor(0, 0, 0)));
  painter->setBrush(QBrush(QColor(255, 255, 255)));

  QString lbl;
  switch (scl) {
  case TimeScale::None:
    lbl = "??";
    break;
  case TimeScale::Decade:
    lbl = d0.toString("yyyy") + QString::fromUtf8("–’")
      + d0.addYears(9).toString("yy");
    break;
  case TimeScale::Year:
    lbl = d0.toString("yyyy");
    break;
  case TimeScale::Month:
    lbl = d0.toString("MMM yyyy");
    break;
  case TimeScale::Day:
    lbl = d0.toString(QString::fromUtf8("MM/dd/’yy"));
    break;
  case TimeScale::Hour:
    lbl = d0.toString("h ap");
    break;
  case TimeScale::DecaMinute:
    lbl = d0.toString("h:mm") + QString::fromUtf8("–")
      + d0.addSecs(9*60).toString(":mm");
    break;
  }
  
  if (r.width() > r.height()) {
    // Horizontal display
    painter->drawText(r, Qt::AlignCenter, lbl);
  } else {
    // Vertical display
    painter->rotate(-90);
    painter->drawText(QRectF(0, -r.width(), r.height(), r.width()),
		      Qt::AlignCenter, lbl);
  }  
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
  switch (scl) {
  case TimeScale::None: {
    d0 = t0;
  } break;
  case TimeScale::Decade: {
    int yr = t0.date().year();
    while (yr%10 != 1)
      yr--;
    d0 = QDateTime(QDate(yr, 1, 1), QTime(0, 0, 0));
  } break;
  case TimeScale::Year: {
    int yr = t0.date().year();
    d0 = QDateTime(QDate(yr, 1, 1), QTime(0, 0, 0));
  } break;
  case TimeScale::Month: {
    int yr = t0.date().year();
    int mo = t0.date().month();
    d0 = QDateTime(QDate(yr, mo, 1), QTime(0, 0, 0));
  } break;
  case TimeScale::Day:
    d0 = QDateTime(t0.date(), QTime(0, 0, 0));
  case TimeScale::Hour: {
    int h = t0.time().hour();
    d0 = QDateTime(t0.date(), QTime(h, 0, 0));
  } break;
  case TimeScale::DecaMinute: {
    int h = t0.time().hour();
    int dm = t0.time().minute()/10;
    d0 = QDateTime(t0.date(), QTime(h, 10*dm, 0));
  } break;
  }
  clearContents();
  rebuildContents();
}

void Filmstrip::setArrangement(Arrangement arr1) {
  arr = arr1;
  for (auto f: subStrips)
    f->setArrangement(arr1);
  relayout();
}

void Filmstrip::setTileSize(int pix) {
  tilesize = pix;
  for (auto f: subStrips)
    f->setTileSize(pix);
  for (auto s: slides)
    s->setTileSize(pix);
  relayout();
}

void Filmstrip::setRowWidth(int pix) {
  rowwidth = pix;
  for (auto f: subStrips)
    f->setRowWidth(pix);
  relayout();
}

void Filmstrip::expand() {
  expanded = true;
  relayout();
}

void Filmstrip::collapse() {
  expanded = false;
  relayout();
}

void Filmstrip::expandAll() {
  for (auto f: subStrips)
    f->expandAll();
  expand();
}

void Filmstrip::collapseAll() {
  for (auto f: subStrips)
    f->collapseAll();
  collapse();
}

void Filmstrip::relayout() {
  if (!expanded)
    return;
  
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
    QRectF r = boundingRect();
    int x = boundingRect().right();
    int y = 0;
    int dy = r.height();
    for (auto c: orderedChildren) {
      Filmstrip *s = dynamic_cast<Filmstrip *>(c);
      QRectF r1 = s ? s->netBoundingRect() : c->boundingRect();
      if (x>0 && x+r1.width()>rowwidth) {
	x = 0;
	y += dy;
	dy = 0;
      }
      c->setPos(x, y);
      x += r1.width();
      if (r1.height()>dy)
	dy = r1.height();
    }
  } break;
  }
  update();
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
  qDebug() << "countinrange";
  if (!q.exec() || !q.next())
    throw q;
  qDebug() << "  counted versions: " << q.value(0).toInt();
  return q.value(0).toInt();
}

bool Filmstrip::anyInRange(QDateTime begin, QDateTime end) const {
  QSqlQuery q(*db);
  q.prepare("select count(*)"
	    " from photos"
	    " where capturedate>=:a and capturedate<:b");
  q.bindValue(":a", begin);
  q.bindValue(":b", end);
  qDebug() << "anyinrange";
  if (!q.exec() || !q.next())
    throw q;
  qDebug() << "  counted photos: " << q.value(0).toInt();
  return q.value(0).toInt() > 0;
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
  if (!q.exec())
    throw q;
  QList<quint64> vv;
  while (q.next())
    vv << q.value(0).toULongLong();
  return vv;
}

void Filmstrip::rebuildContents() {
  int n = countInRange(startDateTime(), endDateTime());

  orderedChildren.clear(); // will rebuild

  if ((n>=MAXDIRECT && scl != TimeScale::DecaMinute)
      || !subStrips.isEmpty()) {
    rebuildContentsWithSubstrips();
    showheader = subStrips.size() > 1;
  } else {
    rebuildContentsDirectly();
    showheader = true;
  }
  
  relayout();
}

void Filmstrip::rebuildContentsWithSubstrips() {
  for (auto s: slides) {
    // removeItem(s);
    delete s;
  }
    
  TimeScale subs =
    scl==TimeScale::Decade ? TimeScale::Year
    : scl==TimeScale::Year ? TimeScale::Month
    : scl==TimeScale::Month ? TimeScale::Day
    : scl==TimeScale::Day ? TimeScale::Hour
    : TimeScale::DecaMinute;
  QDateTime t = startDateTime();
  QDateTime end = endDateTime();

  while (t<end) {
    QDateTime t1 = endFor(t, subs);
    if (t1==t)
      throw QDateTime(); // should never happen
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
	subStrips[t]->collapse();
	subStrips[t]->setArrangement(arr);
	subStrips[t]->setTileSize(tilesize);
	subStrips[t]->setRowWidth(rowwidth);
	connect(subStrips[t], SIGNAL(resized()),
		this, SLOT(relayout()), Qt::QueuedConnection);
	// This could cause excessive calls to relayout--I need to check
      }
      subStrips[t]->setTimeRange(t, subs);
      orderedChildren << subStrips[t];
    } else {
      if (subStrips.contains(t)) {
	// removeItem(subStrips[t]);
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
  QSet<quint64> keep;
  QList<quint64> vv = versionsInRange(startDateTime(), endDateTime());
  for (auto id: vv) {
    if (!slides.contains(id)) {
      slides[id] = new Slide(id, this);
      slides[id]->setTileSize(tilesize);
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
  expandAll();
}

void Filmstrip::mousePressEvent(QGraphicsSceneMouseEvent *) {
}

void Filmstrip::mouseReleaseEvent(QGraphicsSceneMouseEvent *) {
}
