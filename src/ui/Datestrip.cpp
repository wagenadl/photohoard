// Datestrip.cpp

#include "Datestrip.h"
#include "Slidestrip.h"
#include <QSet>
#include <QDebug>
#include "Slide.h"

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
  if (!isExpanded())
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
  if (isExpanded())
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
  connect(s,
          SIGNAL(pressed(quint64, Qt::MouseButton, Qt::KeyboardModifiers)),
          this,
          SIGNAL(pressed(quint64, Qt::MouseButton, Qt::KeyboardModifiers)));
  connect(s, SIGNAL(clicked(quint64, Qt::MouseButton, Qt::KeyboardModifiers)),
          this,
          SIGNAL(clicked(quint64, Qt::MouseButton, Qt::KeyboardModifiers)));
  connect(s,
          SIGNAL(doubleClicked(quint64,
                               Qt::MouseButton, Qt::KeyboardModifiers)),
          this,
          SIGNAL(doubleClicked(quint64,
                               Qt::MouseButton, Qt::KeyboardModifiers)));
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
  mustRebuild = false;

  rebuilding++;

  TimeScale subs = subScale();
  QDateTime end = endDateTime();
  QDateTime t = firstDateInRange(startDateTime(), end);

  if (t.isNull()) {
    rebuilding--;
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

    if (!stripMap.contains(t)) {
      stripMap[t] = newSubstrip(t, subs);
    }
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
}


void Datestrip::expand() {
  Strip::expand();
  if (mustRebuild)
    rebuildContents();
  if (mustRelayout)
    relayout();

  for (auto s: stripOrder)
    s->show();
}

void Datestrip::collapse() {
  for (auto s: stripOrder) {
    if (s->isExpanded()) 
      s->collapse();
    s->hide();
  }
  Strip::collapse();
}

void Datestrip::expandAll() {
  rebuilding++;

  expand();
  for (auto s: stripOrder)
    s->expandAll();

  rebuilding--;

  if (mustRelayout)
    relayout();
}


void Datestrip::relayout() {
  if (!isExpanded()) {
    recalcLabelRect();
    mustRelayout = true;
    return;
  } else if (rebuilding>0) {
    mustRelayout = true;
    return;
  }
  mustRelayout = false;

  Strip::relayout();

  switch (arr) {
  case Arrangement::Horizontal: {
    int x = labelBoundingRect().right();
    for (auto s: stripOrder) {
      s->setPos(x, 0);
      x += s->netBoundingRect().width();
    }
  } break;
  case Arrangement::Vertical: {
    int y = labelBoundingRect().bottom();
    for (auto s: stripOrder) {
      s->setPos(0, y);
      y += s->netBoundingRect().height();
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
  recalcLabelRect();
  emit resized();
}

class Slide *Datestrip::slideByVersion(quint64 vsn) {
  if (mustRebuild || mustRelayout || !expanded)
    return NULL; // is this really necessary?
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
  relayout();
}
  
void Datestrip::setTileSize(int pix) {
  Strip::setTileSize(pix);
  for (auto s: stripOrder)
    s->setTileSize(pix);
  relayout();
}

void Datestrip::setRowWidth(int pix) {
  Strip::setRowWidth(pix);
  int subwidth = subRowWidth(pix);
  for (auto s: stripOrder) 
    s->setRowWidth(subwidth);
  relayout();
}

Strip *Datestrip::stripByDate(QDateTime d, TimeScale s) {
  Strip *a = Strip::stripByDate(d, s);
  if (a)
    return a;

  for (Strip *a0: stripMap) {
    Strip *a = a0->stripByDate(d, s);
    if (a)
      return a;
  }

  return NULL;
}

quint64 Datestrip::versionAt(quint64 vsn, QPoint dcr) {
  qDebug() << "Datestrip " << this << "version at" << vsn << dcr << mustRebuild << mustRelayout << expanded;
  if (mustRebuild || mustRelayout || !expanded)
    return 0; // is this really necessary? Or should we rebuild?
  
  /* The following search algorithm isn't particularly fast, but since
     I think this method will only be called in direct response to key
     presses, that is not a problem.
     The algorithm is simple:
     If the version is not contained in this strip, we return zero.
     Otherwise, we find out which of our substrips contains the version.
     We then ask that substrip to provide the answer. If it can, great.
     Otherwise, we ask the neighboring substrip for its last/first.
   */
  int k = stripNumberContaining(vsn);
  if (k<0)
    return 0;
  quint64 v2 = stripOrder[k]->versionAt(vsn, dcr);
  if (v2>0)
    return v2;

  /* Easy cases exhausted. Let's see. */
  switch (arr) {
  case Arrangement::Horizontal:
    if (dcr.y()) {
      return 0;
    } else if (dcr.x()<0) {
      while (k>0) {
        quint64 r = stripOrder[--k]->lastExpandedVersion();
        if (r)
          return r;
      }
      return 0;
    } else if (dcr.x()>0) {
      while (k<stripOrder.size()-1) {
        quint64 r = stripOrder[++k]->firstExpandedVersion();
        if (r)
          return r;
      }
      return 0;
    } else {
      return 0;
    }
  case Arrangement::Vertical:
    if (dcr.x()) {
      return 0;
    } else if (dcr.y()<0) {
      while (k>0) {
        quint64 r = stripOrder[--k]->lastExpandedVersion();
        if (r)
          return r;
      }
      return 0;
    } else if (dcr.y()>0) {
      while (k<stripOrder.size()-1) {
        quint64 r = stripOrder[++k]->firstExpandedVersion();
        if (r)
          return r;
      }
      return 0;
    } else {
      return 0;
    }
  case Arrangement::Grid:
    if (dcr.x()<0) {
      while (k>0) {
        quint64 r = stripOrder[--k]->lastExpandedVersion();
        if (r)
          return r;
      }
      return 0;
    } else if (dcr.x()>0) {
      while (k<stripOrder.size()-1)  {
        quint64 r = stripOrder[++k]->firstExpandedVersion();
        if (r)
          return r;
      }
      return 0;
    } else {
      // Now we need to deal with vertical displacement in the grid.
      /* Let's first find out if there is any version at all above/below
         our "home" version.
      */
      int ktgt = k;
      quint64 v2 = 0;
      if (dcr.y()<0) {
        while (ktgt>0) {
          v2 = stripOrder[--ktgt]->lastExpandedVersion();
          if (v2)
            break;
        }
      } else if (dcr.y()>0) {
        while (ktgt<stripOrder.size()-1) {
          v2 = stripOrder[++ktgt]->firstExpandedVersion();
          if (v2)
            break;
        }
      }
      qDebug() << "v2: " << v2;
      if (!v2)
        return 0;
      
      /* We know that the "home" version is in stripOrder[k]. Let's find
         out what its coordinates are.
      */
      Slide *homes = stripOrder[k]->slideByVersion(vsn);
      Q_ASSERT(homes);
      Slidestrip *homeparent = dynamic_cast<Slidestrip*>(homes->parentItem());
      // Yes, that's pretty ugly. I should probably improve my infrastructure.
      Q_ASSERT(homeparent);
      int gridx = homeparent->gridPosition(vsn).x();
      qDebug() << "gridx: " << gridx;

      /* Now let's find out the coordinates of our preliminary target */
      Slide *tgts = stripOrder[ktgt]->slideByVersion(v2);
      Q_ASSERT(tgts);
      Slidestrip *tgtparent = dynamic_cast<Slidestrip*>(tgts->parentItem());
      // No prettier the second time around.
      Q_ASSERT(tgtparent);
      int gridy = tgtparent->gridPosition(v2).y();
      qDebug() << "gridy: " << gridy;
      
      /* Ideally, I'd like to go to (gridx, gridy). But that may not exist. */
      while (gridx>=0) {
        quint64 vtgt = tgtparent->versionAt(QPoint(gridx, gridy));
        if (vtgt) {
          qDebug() << "vtgt" << vtgt;
          return vtgt;
        }
        gridx--;
      }      
      return 0;
    }
  }
  return 0; // not executed?
}

quint64 Datestrip::firstExpandedVersion() {
  for (int k=0; k<stripOrder.size(); k++) {
    if (stripOrder[k]->isExpanded()) {
      quint64 res = stripOrder[k]->firstExpandedVersion();
      if (res)
        return res;
    }
  }
  return 0;
}
 
quint64 Datestrip::lastExpandedVersion() {
  for (int k=stripOrder.size()-1; k>=0; --k) {
    if (stripOrder[k]->isExpanded()) {
      quint64 res = stripOrder[k]->lastExpandedVersion();
      if (res)
        return res;
    }
  }
  return 0;
}

Strip *Datestrip::firstExpandedStrip() {
  for (int k=0; k<stripOrder.size(); k++) {
    if (stripOrder[k]->isExpanded()) {
      Strip *res = stripOrder[k]->firstExpandedStrip();
      if (res)
        return res;
    }
  }
  return NULL;
}
 
Strip *Datestrip::lastExpandedStrip() {
  for (int k=stripOrder.size()-1; k>=0; --k) {
    if (stripOrder[k]->isExpanded()) {
      Strip *res = stripOrder[k]->lastExpandedStrip();
      if (res)
        return res;
    }
  }
  return NULL;
}
  
int Datestrip::stripNumberContaining(quint64 vsn) {
  for (int k=0; k<stripOrder.size(); k++)
    if (stripOrder[k]->slideByVersion(vsn))
      return k;
  return -1;
}
