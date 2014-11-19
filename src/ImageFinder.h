// ImageFinder.h

#ifndef IMAGEFINDER_H

#define IMAGEFINDER_H

#include <QObject>
#include <QThread>
#include "Exif.h"

class ImageFinder: public QObject {
  Q_OBJECT;
public:
  ImageFinder(QObject *parent=0);
  virtual ~ImageFinder();
  void setMaxDim(int);
  int queueLength() const { return queuelength; }
public slots:
  void findImage(quint64 id, QString path, int ver, QString ext,
                 Exif::Orientation orient);
signals:
  void foundImage(quint64, QImage);
  void exception(QString);
private slots:
  void handleFoundImage(quint64 id, QImage img);
signals:  // private
  void forwardFindImage(quint64 id, QString path, int ver, QString ext,
                        Exif::Orientation orient, int maxdim);
private:
  QThread thread;
  class IF_Worker *worker;
  int maxdim;
  int queuelength;
};

#endif
