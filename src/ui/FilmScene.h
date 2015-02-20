// FilmScene.h

#ifndef FILMSCENE_H

#define FILMSCENE_H

#include <QGraphicsScene>
#include <QMap>
#include <QMultiMap>
#include "PhotoDB.h"
#include "Image16.h"

class FilmScene: public QGraphicsScene {
  Q_OBJECT;
public:
  FilmScene(PhotoDB const &db, QObject *parent=0);
  virtual ~FilmScene();
  void markSlideFor(quint64, class Slide *);
  void dropSlideFor(quint64);
  void addHeaderFor(quint64, class Strip *);
  void dropHeaderFor(quint64, class Strip *);
public slots:
  void updateImage(quint64, Image16);
signals:
  void pressed(Qt::MouseButton, Qt::KeyboardModifiers);
protected:
  virtual void mousePressEvent(QGraphicsSceneMouseEvent *);
private:
  PhotoDB const &db;
  QMap<quint64, class Slide *> slidemap;
  QMultiMap<quint64, class Strip *> headermap;
};

#endif
