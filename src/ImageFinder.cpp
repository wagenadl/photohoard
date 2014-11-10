// ImageFinder.cpp

#include "ImageFinder.h"
#include "IF_Worker.h"

ImageFinder::ImageFinder(QObject *parent): QObject(parent) {
  maxdim = 0; // i.e., infinity
  queuelength = 0;
  worker = new IF_Worker(this);
  worker->moveToThread(&thread);
  connect(&thread, SIGNAL(finished()), worker, SLOT(deleteLater()));
  connect(this, SIGNAL(forwardFindImage(quint64, QString, int, QString,
                                        Exif::Orientation, int)),
          worker, SLOT(findImage(quint64, QString, int, QString,
                                 Exif::Orientation, int)));
  connect(worker, SIGNAL(foundImage(quint64, QImage)),
          this, SLOT(handleFoundImage(quint64, QImage)));
}

ImageFinder::~ImageFinder() {
  thread.quit();
  thread.wait();
}

void ImageFinder::setMaxDim(int m) {
  maxdim = m;
}

void ImageFinder::findImage(quint64 id, QString path, int ver, QString ext,
                            Exif::Orientation orient) {
  queuelength++;
  emit forwardFindImage(id, path, ver, ext, orient, maxdim);
}

void ImageFinder::handleFoundImage(quint64 id, QImage img) {
  queuelength--;
  emit foundImage(id, img);
}
