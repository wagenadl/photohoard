// IF_Worker.h

#ifndef IF_WORKER_H

#define IF_WORKER_H

#include <QObject>
#include "Exif.h"

class IF_Worker: public QObject {
  Q_OBJECT;
public:
  IF_Worker(QObject *parent=0);
public slots:
  void findImage(quint64 id, QString path, QString mods, QString ext,
                 Exif::Orientation orient, int maxdim, QSize ns);
signals:
  void foundImage(quint64 id, QImage img, bool isFullSize);
  void exception(QString);
private:
  QImage upsideDown(QImage &);
  QImage rotateCW(QImage &);
  QImage rotateCCW(QImage &);
};

#endif
