// Slidestrip.cpp

#include "Slidestrip.h"
#include <QSet>
#include "Slide.h"
#include "PDebug.h"
#include "MetaInfo.h"

#define THRESHOLD 100

static uint qHash(QPoint p) {
  return qHash(p.x()) ^ qHash(p.y() ^ 23478912361);
}

Slidestrip::Slidestrip(PhotoDB const &db, QGraphicsItem *parent):
  Strip(db, parent) {
  hasLatent = false;
  mustRelayout = false;
  mustRebuild = false;
}

Slidestrip::~Slidestrip() {
}


QRectF Slidestrip::subBoundingRect() const {
  if (!expanded)
    return QRectF();
  int nslides = hasLatent ? latentVersions.size() : slideOrder.size();
  switch (arr) {
  case Arrangement::Horizontal: 
    return QRectF(labelHeight(tilesize), 0, nslides*tilesize, tilesize);
  case Arrangement::Vertical:
    return QRectF(0, labelHeight(tilesize), tilesize, nslides*tilesize);
  case Arrangement::Grid: {
    bool horh = hasTopLabel();
    int lw = (horh || !hasheader) ? 0 : labelHeight(tilesize);
    int lh = (horh && hasheader) ? labelHeight(tilesize) : 0;
    int perrow = (rowwidth - lw) / tilesize;
    if (perrow<1)
      perrow = 1;
    int rows = (nslides+perrow-1)/perrow;
    if (rows==1)
      return QRectF(QPointF(lw, lh), QSizeF(tilesize*nslides, tilesize*rows));
    else
      return QRectF(QPointF(lw, lh), QSizeF(tilesize*perrow, tilesize*rows));
  }
  }
  return QRectF(); // not executed
}

void Slidestrip::paint(QPainter *painter,
		       const QStyleOptionGraphicsItem *option,
		       QWidget *widget) {
  Strip::paint(painter, option, widget);
  if (hasLatent)
    instantiate();
}

void Slidestrip::slidePressed(quint64 id,
                              Qt::MouseButton b, Qt::KeyboardModifiers m) {
  emit pressed(id, b, m);
}

void Slidestrip::slideClicked(quint64 id,
                              Qt::MouseButton b, Qt::KeyboardModifiers m) {
  emit clicked(id, b, m);
}

void Slidestrip::slideDoubleClicked(quint64 id,
                              Qt::MouseButton b, Qt::KeyboardModifiers m) {
  emit doubleClicked(id, b, m);
}

Slide *Slidestrip::slideByVersion(quint64 vsn) {
  if (latentVersions.contains(vsn))
    instantiate();
  return slideMap.contains(vsn) ? slideMap[vsn] : 0;
}

void Slidestrip::clearContents() {
  for (auto s: slideOrder) 
    delete s;
  slideOrder.clear();
  slideMap.clear();
}

void Slidestrip::rebuildContents() {
  if (!expanded) {
    mustRebuild = true;
    return;
  }

  mustRebuild = false;
  prepareGeometryChange();

  switch (org) {
  case Organization::ByDate:
    latentVersions = db.versionsInDateRange(startDateTime(), endDateTime());
    if (latentVersions.size()>THRESHOLD && scl<TimeScale::DecaMinute) {
      emit overfilled(d0);
      while (latentVersions.size()>THRESHOLD) {
        latentVersions.takeLast();
      }
    }
    break;
  case Organization::ByFolder:
    latentVersions = db.versionsInFolder(pathname);
    break;
  }
  
  hasLatent = true;
  for (auto s: slideOrder)
    s->hide();
  
  recalcLabelRect(); // hmmm
  update();
}

void Slidestrip::instantiate() {
  prepareGeometryChange();
  QSet<quint64> keep;
  slideOrder.clear();
  
  for (auto id: latentVersions) {
    if (!slideMap.contains(id)) {
      slideMap[id] = new Slide(id, this);
      slideMap[id]->setTileSize(tilesize);
    }
    slideMap[id]->setPos(1e6, 1e6);
    if (expanded) {
      slideMap[id]->show();
      QString txt = MetaInfo(db, id).html();
      slideMap[id]->setToolTip(MetaInfo(db, id).html());
    } else {
      slideMap[id]->hide();
    }
    slideOrder << slideMap[id];
    keep << id;
  }

  for (auto id: slideMap.keys()) {
    if (!keep.contains(id)) {
      delete slideMap[id];
      slideMap.remove(id);
    }
  }

  hasLatent = false;
  latentVersions.clear();
  
  relayout();
}

void Slidestrip::expand() {
  QRectF bb0 = oldbb;
  Strip::expand();

  if (hasLatent)
    return;

  if (mustRebuild)
    rebuildContents();
  else if (mustRelayout)
    relayout();
  
  for (auto s: slideOrder) {
    QString txt = MetaInfo(db, s->version()).html();
    qDebug() << txt;
    s->setToolTip(txt);
    s->show();
  }

  recalcLabelRect();
  QRectF bb1 = netBoundingRect();
  if (bb1!=bb0) {
    oldbb = bb1;
    emit resized();
  }
  update();
}

void Slidestrip::collapse() {
  QRectF bb0 = netBoundingRect();
  Strip::collapse();

  for (auto s: slideOrder)
    s->hide();

  recalcLabelRect();
  QRectF bb1 = netBoundingRect();
  if (bb1!=bb0) {
    oldbb = bb1;
    emit resized();
  }
  update();
}


quint64 Slidestrip::versionAt(QPoint cr) {
  if (revplace.contains(cr))
    return revplace[cr];
  else
    return 0;
}

QPoint Slidestrip::gridPosition(quint64 vsn) {
  if (latentVersions.contains(vsn))
    instantiate();
  if (placement.contains(vsn))
    return placement[vsn];
  else
    return QPoint(-1,-1);
}

quint64 Slidestrip::versionAt(quint64 vsn, QPoint dcr) {
  if (latentVersions.contains(vsn))
    instantiate();
  if (!placement.contains(vsn))
    return 0;
  QPoint cr = placement[vsn] + dcr;
  if (cr.x()<0) 
    cr = QPoint(maxcplace, cr.y()-1);
  else if (cr.x()>maxcplace)
    cr = QPoint(0, cr.y()+1);
  if (revplace.contains(cr))
    return revplace[cr];
  if (arr==Arrangement::Grid && dcr.y()>0) {
    // Going down, we may be going past the final version
    while (cr.x()>0) {
      cr.setX(cr.x()-1);
      if (revplace.contains(cr))
        return revplace[cr];
    }
  }
  return 0;
}

void Slidestrip::relayout() {
  QRectF bb0 = oldbb;
  
  placement.clear();
  revplace.clear();
  if (hasLatent) {
    Strip::relayout();
    return;
  }
  
  if (!expanded) {
    recalcLabelRect();
    mustRelayout = true;
    return;
  }

  Strip::relayout();

  QMap<Slide *, quint64> revmap;
  for (auto it=slideMap.begin(); it!=slideMap.end(); ++it)
    revmap[it.value()] = it.key();
  
  switch (arr) {
  case Arrangement::Horizontal: {
    int x = labelHeight(tilesize);
    int k = 0;
    for (auto s: slideOrder) {
      s->setPos(x, 0);
      placement[revmap[s]] = QPoint(k, 0);
      x += tilesize;
      k++;
    }
  } break;
  case Arrangement::Vertical: {
    int y = labelHeight(tilesize);
    int k = 0;
    for (auto s: slideOrder) {
      s->setPos(0, y);
      placement[revmap[s]] = QPoint(0, k);
      y += tilesize;
      k++;
    }
  } break;
  case Arrangement::Grid: {
    bool htl = hasTopLabel();
    int x0 = (htl || !hasheader) ? 0 : labelHeight(tilesize);
    int y0 = (htl && hasheader) ? labelHeight(tilesize) : 0;
    bool atstart = true;
    int x = x0;
    int y = y0;
    int k = 0;
    int l = 0;
    for (auto s: slideOrder) {
      if (x+tilesize>rowwidth && !atstart) {
	y += tilesize;
        k = 0;
        l++;
	x = x0;
	atstart = true;
      }
      s->setPos(x, y);
      placement[revmap[s]] = QPoint(k, l);
      x += tilesize;
      k++;
      atstart = false;
    }
    break;
  }
  }
  maxcplace = 0;
  for (auto it=placement.begin(); it!=placement.end(); ++it) {
    if (it.value().x() > maxcplace)
      maxcplace = it.value().x();
    revplace[it.value()] = it.key();
  }
  recalcLabelRect();

  QRectF bb1 = netBoundingRect();
  if (bb1!=bb0) {
    oldbb = bb1;
    emit resized();
  }
  update();
}

  
void Slidestrip::setArrangement(Strip::Arrangement arr) {
  Strip::setArrangement(arr);
  relayout();
}
  
void Slidestrip::setTileSize(int pix) {
  Strip::setTileSize(pix);
  for (auto s: slideOrder)
    s->setTileSize(pix);
  relayout();
}

void Slidestrip::setRowWidth(int pix) {
  Strip::setRowWidth(pix);
  relayout();
}

quint64 Slidestrip::firstExpandedVersion() {
  if (!expanded)
    return 0;
  if (slideOrder.isEmpty())
    return 0;
  Slide *s = slideOrder.first();
  Q_ASSERT(s);
  return s->version();
}

quint64 Slidestrip::lastExpandedVersion() {
  if (!expanded)
    return 0;
  if (slideOrder.isEmpty())
    return 0;
  Slide *s = slideOrder.last();
  Q_ASSERT(s);
  return s->version();
}

Strip *Slidestrip::firstExpandedStrip() {
  return this;
}

Strip *Slidestrip::lastExpandedStrip() {
  return this;
}

