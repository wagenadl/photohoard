// FilmScene.cpp

#include "FilmScene.h"
#include "Slide.h"
#include "Strip.h"
#include <QGraphicsSceneMouseEvent>

FilmScene::FilmScene(PhotoDB *db, QObject *parent):
  QGraphicsScene(parent), db(db) {
  setBackgroundBrush(QColor(128, 128, 128));
}

FilmScene::~FilmScene() {
}

void FilmScene::markSlideFor(quint64 id, class Slide *s) {
  slidemap[id] = s;
}

void FilmScene::dropSlideFor(quint64 id) {
  slidemap.remove(id);
}

void FilmScene::addHeaderFor(quint64 id, class Strip *s) {
  headermap.insert(id, s);
}

void FilmScene::dropHeaderFor(quint64 id, class Strip *s) {
  headermap.remove(id, s);
}

void FilmScene::updateImage(quint64 id, Image16 img) {
  if (slidemap.contains(id))
    slidemap[id]->updateImage(img);
  for (auto s: headermap.values(id))
    s->updateHeader(img);
}


void FilmScene::mousePressEvent(QGraphicsSceneMouseEvent *e) {
  if (itemAt(e->scenePos(), QTransform())==0)
    emit pressed(e->button(), e->modifiers());
  else
    QGraphicsScene::mousePressEvent(e);
}
