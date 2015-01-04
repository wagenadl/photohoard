// Slidestrip.cpp

#include "Slidestrip.h"
#include <QSet>
#include "Slide.h"
#include <QDebug>

#define THRESHOLD 100

Slidestrip::Slidestrip(PhotoDB const &db, QGraphicsItem *parent):
  Strip(db, parent) {
  hasLatent = false;
  mustRelayout = false;
  mustRebuild = false;
}

Slidestrip::~Slidestrip() {
}


QRectF Slidestrip::subBoundingRect() const {
  if (shouldDebug())
    qDebug() << "Slidestrip " << d0 << "(" << int(scl) << "): "
	     << " subBoundingRect: exp=" << expanded
	     << " latent=" << hasLatent;
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
    int lw = horh ? 0 : labelHeight(tilesize);
    int lh = horh ? labelHeight(tilesize) : 0;
    int perrow = (rowwidth - lw) / tilesize;
    if (perrow<1)
      perrow = 1;
    int rows = (nslides+perrow-1)/perrow;
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

void Slidestrip::slidePressed(quint64 id) {
  emit pressed(id);
}

void Slidestrip::slideClicked(quint64 id) {
  emit clicked(id);
}

void Slidestrip::slideDoubleClicked(quint64 id) {
  emit doubleClicked(id);
}

Slide *Slidestrip::slideByVersion(quint64 vsn) const {
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

  if (shouldDebug())
    qDebug() << "Slidestrip " << d0 << "(" << int(scl) << "): "
	     << " rebuild";
  
  mustRebuild = false;
  prepareGeometryChange();
  latentVersions = versionsInRange(startDateTime(), endDateTime());
  if (latentVersions.size()>THRESHOLD && scl<TimeScale::DecaMinute) {
    emit overfilled(d0);
    while (latentVersions.size()>THRESHOLD) {
      latentVersions.takeLast();
    }
  }
  hasLatent = true;
  for (auto s: slideOrder)
    s->hide();
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
    if (expanded)
      slideMap[id]->show();
    else
      slideMap[id]->hide();
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
  Strip::expand();
  if (hasLatent)
    return;

  if (mustRebuild)
    rebuildContents();
  else if (mustRelayout)
    relayout();
  
  for (auto s: slideOrder)
    s->show();
}

void Slidestrip::collapse() {
  Strip::collapse();
  for (auto s: slideOrder)
    s->hide();
}

void Slidestrip::relayout() {
  if (shouldDebug())
    qDebug() << "Slidestrip " << d0 << "(" << int(scl) << "): "
	     << " relayout: exp=" << expanded
	     << " latent=" << hasLatent;
  if (hasLatent)
    return;
  if (!expanded) {
    recalcLabelRect();
    mustRelayout = true;
    return;
  }

  Strip::relayout();
  
  switch (arr) {
  case Arrangement::Horizontal: {
    int x = labelBoundingRect().right();
    for (auto s: slideOrder) {
      s->setPos(x, 0);
      x += tilesize;
    }
  } break;
  case Arrangement::Vertical: {
    int y = labelBoundingRect().bottom();
    qDebug() << "Slidestrip: relayout" << y;
    for (auto s: slideOrder) {
      s->setPos(0, y);
      y += tilesize;
      qDebug() << "  : " << y;
    }
  } break;
  case Arrangement::Grid: {
    bool htl = hasTopLabel();
    int x0 = htl ? 0 : labelHeight(tilesize);
    int y0 = htl ? labelHeight(tilesize) : 0;
    bool atstart = true;
    int x = x0;
    int y = y0;
    for (auto s: slideOrder) {
      if (x+tilesize>rowwidth && !atstart) {
	y += tilesize;
	x = x0;
	atstart = true;
      }
      s->setPos(x, y);
      x += tilesize;
      atstart = false;
    }
    break;
  }
  }
  recalcLabelRect();
  emit resized();
}

  
void Slidestrip::setArrangement(Arrangement arr) {
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
