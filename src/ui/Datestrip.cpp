// Datestrip.cpp

#include "Datestrip.h"
#include "Slidestrip.h"
#include <QSet>
#include "PDebug.h"
#include "Slide.h"

#define MAXDIRECT 50

inline uint qHash(QDateTime const &dt) {
  return qHash(dt.toMSecsSinceEpoch());
}

Datestrip::Datestrip(PhotoDB *db, QGraphicsItem *parent):
  Strip(db, parent) {
  mustRebuild = false;
  mustRelayout = false;
  thisFolderStrip = NULL;
  rebuilding = 0;
}

Datestrip::~Datestrip() {
}


QRectF Datestrip::subBoundingRect() const {
  if (!isExpanded())
    return QRectF();
  if (stripOrder.isEmpty())
    return QRectF();
  Strip *rs = rightMostSubstrip();
  Strip *bs = bottomMostSubstrip();
  double r = rs ? rs->mapToParent(rs->netBoundingRect().bottomRight()).x() : 1;
  double b = bs ? bs->mapToParent(bs->netBoundingRect().bottomRight()).y() : 1;
  return QRectF(QPointF(0, 0), QPointF(r, b));                        
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
  dateMap.clear();
  folderMap.clear();
  thisFolderStrip = NULL;
}

void Datestrip::convertStrip(QDateTime t) {
  if (!dateMap.contains(t))
    return;
  Strip *s = dateMap[t];
  bool e = s->isExpanded();
  TimeScale subs = subScale();
  dateMap[t] = newSubstrip(t, subs);
  for (auto it=stripOrder.begin(); it!=stripOrder.end(); it++) {
    if (*it == s) {
      *it = dateMap[t];
      break;
    }
  }
  delete s;
  dateMap[t]->setTimeRange(t, subs);
  if (isExpanded())
    dateMap[t]->show();
  else
    dateMap[t]->hide();
  if (e)
    dateMap[t]->expand();
  relayout();
}

Strip *Datestrip::newStrip(bool indirect, bool protectoverfill) {
  Strip *s = 0; 
  if (indirect) {
    s = new Datestrip(db, this);
  } else {
    s = new Slidestrip(db, this);
    if (protectoverfill)
      connect(s, SIGNAL(overfilled(QDateTime)),
              this, SLOT(convertStrip(QDateTime)), Qt::QueuedConnection);
  }

  connect(s, SIGNAL(needImage(quint64, QSize)),
          this, SIGNAL(needImage(quint64, QSize)));
  connect(s, SIGNAL(pressed(quint64, Qt::MouseButton,
                            Qt::KeyboardModifiers)),
          this, SIGNAL(pressed(quint64, Qt::MouseButton,
                               Qt::KeyboardModifiers)));
  connect(s, SIGNAL(clicked(quint64, Qt::MouseButton,
                            Qt::KeyboardModifiers)),
          this, SIGNAL(clicked(quint64, Qt::MouseButton,
                               Qt::KeyboardModifiers)));
  connect(s, SIGNAL(doubleClicked(quint64, Qt::MouseButton,
                                  Qt::KeyboardModifiers)),
          this, SIGNAL(doubleClicked(quint64, Qt::MouseButton,
                                     Qt::KeyboardModifiers)));

  s->setArrangement(arr);
  s->setTileSize(tilesize);
  s->setRowWidth(subRowWidth(rowwidth));

  connect(s, SIGNAL(resized()), this, SLOT(relayout()));

  return s;
}

Strip *Datestrip::newSubstrip(QDateTime t, Strip::TimeScale subs) {
  int n = db->countInDateRange(t, endFor(t, subs));
  bool indirect = n>=MAXDIRECT && subs!=TimeScale::DecaMinute;
  Strip *s = newStrip(indirect, true);
  return s;
}

void Datestrip::rebuildContents() {
  if (!expanded) {
    mustRebuild = true;
    return;
  }
  mustRebuild = false;

  switch (org) {
  case Organization::ByDate:
    rebuildByDate();
    break;
  case Organization::ByFolder:
    rebuildByFolder();
    break;
  }
}

void Datestrip::rebuildByFolder() {
  rebuilding++;

  bool anyhere = db->countInFolder(pathname)>0;
  bool anybelow = db->anyInTreeBelow(pathname);

  if (!anyhere && !anybelow) {
    rebuilding--;
    clearContents();
    return;
  }

  for (auto s: dateMap)
    delete s;
  dateMap.clear();
  stripOrder.clear();

  QSet<QString> keep;

  if (anyhere) {
    if (!thisFolderStrip) {
      thisFolderStrip = newStrip(false, false);
      thisFolderStrip->setFolder(pathname);
      thisFolderStrip->setDisplayName("");
      if (!anybelow)
        thisFolderStrip->makeHeaderless();
    }
    if (expanded) {
      if (!anybelow)
        thisFolderStrip->expand();
      thisFolderStrip->show();
    } else {
      thisFolderStrip->hide();
    }
    stripOrder << thisFolderStrip;
  } else {
    delete thisFolderStrip;
    thisFolderStrip = NULL;
  }

  if (anybelow) {
    QList<QString> fff = db->subFolders(pathname);
    for (QString f: fff) {
      if (db->countInFolder(f)==0 && !db->anyInTreeBelow(f))
        continue;

      keep << f;
      
      if (!folderMap.contains(f)) {
        folderMap[f] = newStrip(true, false);
      }
      Strip *s = folderMap[f];
      s->setFolder(f);
      stripOrder << s;
      if (expanded)
        s->show();
      else
        s->hide();
    }
  }

  for (auto id: folderMap.keys()) {
    if (!keep.contains(id)) {
      delete folderMap[id];
      folderMap.remove(id);
    }
  }

  rebuilding--;
  relayout();
}

void Datestrip::rebuildByDate() {  
  rebuilding++;

  TimeScale subs = subScale();
  QDateTime end = endDateTime();
  QDateTime t = db->firstDateInRange(startDateTime(), end);

  if (t.isNull()) {
    rebuilding--;
    clearContents();
    return;
  }

  for (auto s: folderMap)
    delete s;
  delete thisFolderStrip;
  folderMap.clear();
  stripOrder.clear();
  
  QSet<QDateTime> keep;

  while (t.isValid()) {
    t = startFor(t, subs);
    keep << t;
    QDateTime t1 = endFor(t, subs);
    Q_ASSERT(t1>t);

    if (!dateMap.contains(t)) {
      dateMap[t] = newSubstrip(t, subs);
    }
    Strip *s = dateMap[t];
    s->setTimeRange(t, subs);
    stripOrder << s;
    if (expanded)
      s->show();
    else
      s->hide();

    t = db->firstDateInRange(t1, end);
  }

  for (auto id: dateMap.keys()) {
    if (!keep.contains(id)) {
      delete dateMap[id];
      dateMap.remove(id);
    }
  }

  rebuilding--;
  relayout();
}


void Datestrip::expand() {
  QRectF bb0 = oldbb;
  Strip::expand();

  //  pDebug() << "Expand" << d0 << int(scl) << mustRebuild << mustRelayout;
  if (mustRebuild)
    rebuildContents();

  rebuilding ++;
  for (auto s: stripOrder)
    s->show();
  if (thisFolderStrip)
    thisFolderStrip->expand();

  recalcLabelRect();
  rebuilding --;
  
  if (mustRelayout)
    relayout();

  QRectF bb1 = netBoundingRect();
  if (bb1!=bb0) {
    //    pDebug() << "Expand" << d0 << int(scl) << bb0 << bb1 << expanded;
    oldbb = bb1;
    emit resized();
  }

  update();
}

void Datestrip::collapse() {
  QRectF bb0 = oldbb;
  Strip::collapse();

  for (auto s: stripOrder) {
    if (s->isExpanded()) 
      s->collapse();
    s->hide();
  }

  recalcLabelRect();
  QRectF bb1 = netBoundingRect();
  if (bb1!=bb0) {
    //    pDebug() << "Collapse" << d0 << int(scl) << bb0 << bb1 << expanded;
    oldbb = bb1;
    emit resized();
  }

  update();
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

Strip *Datestrip::rightMostSubstrip() const {
  if (!rightmostsub)
    findBottomRight();
  return rightmostsub;
}

Strip *Datestrip::bottomMostSubstrip() const {
  if (!rightmostsub)
    findBottomRight();
  return bottommostsub;
}

void Datestrip::findBottomRight() const {
  double r = -1;
  double b = -1;
  Strip *rs=0, *bs=0;
  for (Strip *s: stripOrder) {
    QPointF br = s->mapToParent(s->netBoundingRect().bottomRight());
    if (rs==0 || br.x()>r) {
      r = br.x();
      rs = s;
    }
    if (bs==0 || br.y()>b) {
      b = br.y();
      bs = s;
    }
  }
  rightmostsub = rs;
  bottommostsub = bs;
}      

void Datestrip::relayout() {
  prepareGeometryChange();

  if (rebuilding>0) {
    mustRelayout = true;
    return;
  }  

  recalcLabelRect();

  if (!isExpanded()) {
    mustRelayout = true;
    return;
  }

  mustRelayout = false;

  QRectF bb0 = oldbb;

  rightmostsub = QPointer<Strip>(0);
  bottommostsub = QPointer<Strip>(0);

  Strip::relayout();

  switch (arr) {
  case Arrangement::Horizontal: {
    int x = labelBoundingRect().right();
    bool dx = false;
    bool suppress = scl!=TimeScale::Eternity;
    for (auto s: stripOrder) {
      if (s->isExpanded() && !suppress)
        dx = true;
      if (dx)
        x += 4;
      s->setPos(x, 0);
      x += s->netBoundingRect().width();
      dx = s->isExpanded();
      suppress = false;
    }
  } break;
  case Arrangement::Vertical: {
    int y = labelBoundingRect().bottom();
    bool dy = false;
    bool suppress = scl!=TimeScale::Eternity;
    for (auto s: stripOrder) {
      if (s->isExpanded() && !suppress)
        dy = true;
      if (dy)
        y += 4;
      s->setPos(0, y);
      y += s->netBoundingRect().height();
      dy = s->isExpanded();
      suppress = false;
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
    if (scl==TimeScale::Eternity)
      y += 2 + edy;
    else if (hasTopLabel())
      y += 1;
    bool atstart = true;

    for (auto s: stripOrder) {
      bool ex = s->isExpanded();
      QRectF r1 = s->netBoundingRect();
      if (x+r1.width()>rowwidth && !atstart) {
	y += dy + edy;
        if (ex)
          y += 2; // extra space before expanded section
	x = x0;
	dy = 0;
	atstart = true;
      }
      
      s->setPos(x, y);
      if (r1.height()>dy)
	dy = r1.height();

      x += r1.width();	
      atstart = false;
    }
  } break;
  }

  recalcLabelRect();
  QRectF bb1 = netBoundingRect();
  if (bb1!=bb0) {
    //    pDebug() << "Relayout" << d0 << int(scl) << bb0 << bb1 << expanded;
    oldbb = bb1;
    emit resized();
  }

  update();
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
  rebuilding ++;
  Strip::setArrangement(arr);
  for (auto s: stripOrder) 
    s->setArrangement(arr);
  rebuilding --;
  relayout();
}
  
void Datestrip::setTileSize(int pix) {
  rebuilding ++;
  Strip::setTileSize(pix);
  for (auto s: stripOrder)
    s->setTileSize(pix);
  rebuilding --;
  relayout();
}

void Datestrip::setRowWidth(int pix) {
  rebuilding ++;
  Strip::setRowWidth(pix);
  int subwidth = subRowWidth(pix);
  for (auto s: stripOrder) 
    s->setRowWidth(subwidth);
  rebuilding --;
  relayout();
}

Strip *Datestrip::stripByDate(QDateTime d, TimeScale s) {
  Strip *a = Strip::stripByDate(d, s);
  if (a)
    return a;

  for (Strip *a0: dateMap) {
    Strip *a = a0->stripByDate(d, s);
    if (a)
      return a;
  }

  return NULL;
}


Strip *Datestrip::stripByFolder(QString path) {
  Strip *a = Strip::stripByFolder(path);
  if (a)
    return a;

  for (Strip *a0: folderMap) {
    Strip *a = a0->stripByFolder(path);
    if (a)
      return a;
  }

  return NULL;
}

quint64 Datestrip::versionAt(quint64 vsn, QPoint dcr) {
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

      /* Now let's find out the coordinates of our preliminary target */
      Slide *tgts = stripOrder[ktgt]->slideByVersion(v2);
      Q_ASSERT(tgts);
      Slidestrip *tgtparent = dynamic_cast<Slidestrip*>(tgts->parentItem());
      // No prettier the second time around.
      Q_ASSERT(tgtparent);
      int gridy = tgtparent->gridPosition(v2).y();
      
      /* Ideally, I'd like to go to (gridx, gridy). But that may not exist. */
      while (gridx>=0) {
        quint64 vtgt = tgtparent->versionAt(QPoint(gridx, gridy));
        if (vtgt) {
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
