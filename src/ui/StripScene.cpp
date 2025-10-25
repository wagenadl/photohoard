// FilmScene.cpp

#include "StripScene.h"
#include "Slide.h"
#include "Strip.h"
#include <QGraphicsSceneMouseEvent>
#include <QDebug>

StripScene::StripScene(PhotoDB *db, QObject *parent):
  QGraphicsScene(parent), db(db) {
  setBackgroundBrush(QColor(160, 160, 160));
}

StripScene::~StripScene() {
}

void StripScene::markSlideFor(quint64 id, class Slide *s) {
  slidemap[id] = s;
}

void StripScene::dropSlideFor(quint64 id) {
  slidemap.remove(id);
}

void StripScene::addHeaderFor(quint64 id, class Strip *s) {
  headermap.insert(id, s);
}

void StripScene::dropHeaderFor(quint64 id, class Strip *s) {
  headermap.remove(id, s);
}

void StripScene::updateImage(quint64 id, Image16 img, bool chgd) {
  if (slidemap.contains(id))
    slidemap[id]->updateImage(img, chgd);
  for (auto s: headermap.values(id))
    s->updateHeader(img, chgd);
}

void StripScene::quickRotate(quint64 id, int dphi) {
  if (slidemap.contains(id))
    slidemap[id]->quickRotate(dphi);
  for (auto s: headermap.values(id))
    s->updateHeaderRotation(dphi);
}


void StripScene::mousePressEvent(QGraphicsSceneMouseEvent *e) {
  if (itemAt(e->scenePos(), QTransform())==0)
    emit pressed(e->button(), e->modifiers());
  else
    QGraphicsScene::mousePressEvent(e);
}
