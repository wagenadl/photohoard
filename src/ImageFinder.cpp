// ImageFinder.cpp

#include "ImageFinder.h"
#include "IF_Worker.h"
#include <QMetaType>

ImageFinder::ImageFinder(QObject *parent): QObject(parent) {
  setObjectName("ImageFinder");
  queuelength = 0;
  worker = new IF_Worker(0);
  worker->moveToThread(&thread);
  qRegisterMetaType<Exif::Orientation>("Exif::Orientation");
  connect(&thread, SIGNAL(finished()), worker, SLOT(deleteLater()));
  connect(this, SIGNAL(forwardFindImage(quint64, QString, int, QString,
                                        Exif::Orientation, int)),
          worker, SLOT(findImage(quint64, QString, int, QString,
                                 Exif::Orientation, int)));
  connect(worker, SIGNAL(foundImage(quint64, QImage)),
          this, SLOT(handleFoundImage(quint64, QImage)));
  connect(worker, SIGNAL(exception(QString)),
	  this, SIGNAL(exception(QString)));
  thread.start();
}

ImageFinder::~ImageFinder() {
  thread.quit();
  thread.wait();
}

void ImageFinder::findImage(quint64 id, QString path, int ver, QString ext,
                            Exif::Orientation orient, int maxdim, QSize ns) {
  queuelength++;
  emit forwardFindImage(id, path, ver, ext, orient, maxdim, ns);
}

void ImageFinder::handleFoundImage(quint64 id, QImage img) {
  queuelength--;
  emit foundImage(id, img);
}
