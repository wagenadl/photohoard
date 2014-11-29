// Datestrip.cpp

#include "Datestrip.h"
#include "Slidestrip.h"
#include <QSet>
#include <QDebug>

#define MAXDIRECT 50

inline uint qHash(QDateTime const &dt) {
  return qHash(dt.toMSecsSinceEpoch());
}

Datestrip::Datestrip(PhotoDB const &db, QGraphicsItem *parent):
  Strip(db, parent) {
  mustRebuild = false;
  mustRelayout = false;
  rebuilding = 0;
}

Datestrip::~Datestrip() {
}


QRectF Datestrip::subBoundingRect() const {
  if (shouldDebug())
    qDebug() << "Datestrip " << d0 << "(" << int(scl) << "): "
	     << "subBoundingRect: exp=" << expanded
	     << " empty=" << stripOrder.isEmpty();
  if (!expanded)
    return QRectF();
  if (stripOrder.isEmpty())
    return QRectF();
  Strip *s = stripOrder.last();
  return QRectF(QPointF(0, 0),
		QPointF(s->mapToParent(s->netBoundingRect().bottomRight())));
}

Strip::TimeScale Datestrip::subScale() const {
  return scl==TimeScale::Eternity ? TimeScale::Decade
    : scl==TimeScale::Decade ? TimeScale::Year
    : scl==TimeScale::Year ? TimeScale::Month
    : scl==TimeScale::Month ? TimeScale::Day
    : scl==TimeScale::Day ? TimeScale::Hour
    : TimeScale::DecaMinute;
}

void Datestrip::clearContents() {
  for (auto s: stripOrder)
    delete s;
  stripOrder.clear();
  stripMap.clear();
}

void Datestrip::convertStrip(QDateTime t) {
  if (!stripMap.contains(t))
    return;
  Strip *s = stripMap[t];
  bool e = s->isExpanded();
  TimeScale subs = subScale();
  stripMap[t] = newSubstrip(t, subs);
  for (auto it=stripOrder.begin(); it!=stripOrder.end(); it++) {
    if (*it == s) {
      *it = stripMap[t];
      break;
    }
  }
  delete s;
  stripMap[t]->setTimeRange(t, subs);
  if (expanded)
    stripMap[t]->show();
  else
    stripMap[t]->hide();
  if (e)
    stripMap[t]->expand();
  relayout();
}  

Strip *Datestrip::newSubstrip(QDateTime t, Strip::TimeScale subs) {
  Strip *s;
  int n = countInRange(t, endFor(t, subs));
  bool indirect = n>=MAXDIRECT && subs!=TimeScale::DecaMinute;
  if (indirect) {
    s = new Datestrip(db, this);
  } else {
    s = new Slidestrip(db, this);
    connect(s, SIGNAL(overfilled(QDateTime)),
            this, SLOT(convertStrip(QDateTime)), Qt::QueuedConnection);
  }
  connect(s, SIGNAL(needImage(quint64, QSize)),
          this, SIGNAL(needImage(quint64, QSize)));
  connect(s, SIGNAL(pressed(quint64)),
          this, SIGNAL(pressed(quint64)));
  connect(s, SIGNAL(clicked(quint64)),
          this, SIGNAL(clicked(quint64)));
  connect(s, SIGNAL(doubleClicked(quint64)),
          this, SIGNAL(doubleClicked(quint64)));
  s->setArrangement(arr);
  s->setTileSize(tilesize);
  s->setRowWidth(subRowWidth(rowwidth));
  connect(s, SIGNAL(resized()), this, SLOT(relayout()));
  return s;
}

void Datestrip::rebuildContents() {
  if (!expanded) {
    mustRebuild = true;
    return;
  }

  if (shouldDebug())
    qDebug() << "Datestrip " << d0 << "(" << int(scl) << "): "
	     << "rebuildContents";
  
  rebuilding++;

  TimeScale subs = subScale();
  QDateTime end = endDateTime();
  QDateTime t = firstDateInRange(startDateTime(), end);

  if (t.isNull()) {
    clearContents();
    return;
  }

  stripOrder.clear();
  QSet<QDateTime> keep;

  while (t.isValid()) {
    t = startFor(t, subs);
    keep << t;
    QDateTime t1 = endFor(t, subs);
    Q_ASSERT(t1>t);

    if (!stripMap.contains(t))
      stripMap[t] = newSubstrip(t, subs);
    Strip *s = stripMap[t];
    s->setTimeRange(t, subs);
    stripOrder << s;
    if (expanded)
      s->show();
    else
      s->hide();

    t = firstDateInRange(t1, end);
  }

  for (auto id: stripMap.keys()) {
    if (!keep.contains(id)) {
      delete stripMap[id];
      stripMap.remove(id);
    }
  }

  rebuilding--;
  relayout();
  if (shouldDebug())
    qDebug() << "Datestrip " << d0 << "(" << int(scl) << "): "
	     << "rebuild done";
}


void Datestrip::expand() {
  Strip::expand();
  if (mustRebuild)
    rebuildContents();
  if (mustRelayout)
    relayout();

  for (auto s: stripOrder)
    s->show();
  if (shouldDebug())
    qDebug() << "Datestrip " << d0 << "(" << int(scl) << "): "
	     << "expand done";
}

void Datestrip::collapse() {
  Strip::collapse();
  for (auto s: stripOrder)
    s->hide();
}

void Datestrip::expandAll() {
  rebuilding++;
  if (shouldDebug())
    qDebug() << "Datestrip " << d0 << "(" << int(scl) << "): "
	     << "expandAll";
  expand();
  for (auto s: stripOrder)
    s->expandAll();
  rebuilding--;
  if (mustRelayout)
    relayout();
  if (shouldDebug())
    qDebug() << "Datestrip " << d0 << "(" << int(scl) << "): "
	     << "expandAll done";
}

void Datestrip::collapseAll() {
  collapse();
  for (auto s: stripOrder)
    s->collapseAll();
}

void Datestrip::relayout() {
  if (!expanded || rebuilding>0) {
    mustRelayout = true;
    return;
  }

  Strip::relayout();
  if (shouldDebug())
    qDebug() << "Datestrip " << d0 << "(" << int(scl) << "): "
	     << "relayout";

  switch (arr) {
  case Arrangement::Horizontal: {
    int x = labelBoundingRect().right();
    for (auto s: stripOrder) {
      s->setPos(x, 0);
      x += netBoundingRect().width();
    }
  } break;
  case Arrangement::Vertical: {
    int y = labelBoundingRect().bottom();
    for (auto s: stripOrder) {
      s->setPos(0, y);
      y += netBoundingRect().height();
    }
  } break;
  case Arrangement::Grid: {
    bool hastoplabel = hasTopLabel();
    int x0 = hastoplabel ? 0 : labelHeight(tilesize);
    int y0 = hastoplabel ? labelHeight(tilesize) : 0;
    int edy = 2;
    int dy = 0;
    int x = x0;
    int y = y0;
    bool atstart = true;
    for (auto s: stripOrder) {
      bool ex = s->isExpanded();
      QRectF r1 = s->netBoundingRect();
      if ((ex || x+r1.width()>rowwidth) && !atstart) {
	y += dy + edy;
	x = x0;
	dy = 0;
	atstart = true;
      }
      
      s->setPos(x, y);
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
  if (shouldDebug())
    qDebug() << "Datestrip " << d0 << "(" << int(scl) << "): "
	     << "relayout done";
}

class Slide *Datestrip::slideByVersion(quint64 vsn) const {
  for (auto f: stripOrder) {
    Slide *s = f->slideByVersion(vsn);
    if (s)
      return s;
  }
  return NULL;
}

void Datestrip::setArrangement(Arrangement arr) {
  Strip::setArrangement(arr);
  for (auto s: stripOrder) 
    s->setArrangement(arr);
}
  
void Datestrip::setTileSize(int pix) {
  Strip::setTileSize(pix);
  for (auto s: stripOrder)
    s->setTileSize(pix);
}

void Datestrip::setRowWidth(int pix) {
  Strip::setRowWidth(pix);
  int subwidth = subRowWidth(pix);
  for (auto s: stripOrder) 
    s->setRowWidth(subwidth);
}
