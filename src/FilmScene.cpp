// FilmScene.cpp

#include "FilmScene.h"
#include "Slide.h"
#include "Strip.h"

FilmScene::FilmScene(PhotoDB const &db, QObject *parent):
  QGraphicsScene(parent), db(db) {
  setBackgroundBrush(QColor(255, 255, 255));
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

void FilmScene::updateImage(quint64 id, QImage img) {
  if (slidemap.contains(id))
    slidemap[id]->updateImage(img);
  for (auto s: headermap.values(id))
    s->updateHeader(img);
}


