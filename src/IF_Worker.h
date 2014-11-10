// IF_Worker.h

#ifndef IF_WORKER_H

#define IF_WORKER_H

#include <QObject>
#include "Exif.h"

class IF_Worker: public QObject {
  Q_OBJECT;
public:
  IF_Worker(QObject *parent=0): QObject(parent) {}
public slots:
  void findImage(quint64 id, QString path, int ver, QString ext,
                 Exif::Orientation orient, int maxdim);
signals:
  void foundImage(quint64 id, QImage img);
private:
  void upsideDown(QImage &);
  QImage rotateCW(QImage const &);
  QImage rotateCCW(QImage const &);
};

#endif
