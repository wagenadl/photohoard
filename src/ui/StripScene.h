// StripScene.h

#ifndef STRIPSCENE_H

#define STRIPSCENE_H

#include <QGraphicsScene>
#include <QMap>
#include <QMultiMap>
#include "PhotoDB.h"
#include "Image16.h"

class StripScene: public QGraphicsScene {
  Q_OBJECT;
public:
  StripScene(PhotoDB *db, QObject *parent=0);
  virtual ~StripScene();
  void markSlideFor(quint64, class Slide *);
  void dropSlideFor(quint64);
  void addHeaderFor(quint64, class Strip *);
  void dropHeaderFor(quint64, class Strip *);
public slots:
  void updateImage(quint64, Image16);
  void quickRotate(quint64, int dphi);
signals:
  void pressed(Qt::MouseButton, Qt::KeyboardModifiers);
protected:
  virtual void mousePressEvent(QGraphicsSceneMouseEvent *);
private:
  PhotoDB *db;
  QMap<quint64, class Slide *> slidemap;
  QMultiMap<quint64, class Strip *> headermap;
};

#endif
