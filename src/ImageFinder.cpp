// ImageFinder.cpp

#include "ImageFinder.h"
#include "IF_Worker.h"
#include <QMetaType>
#include <QDebug>

ImageFinder::ImageFinder(QObject *parent): QObject(parent) {
  setObjectName("ImageFinder");
  queuelength = 0;
  worker = new IF_Worker(0);
  worker->moveToThread(&thread);
  qRegisterMetaType<Exif::Orientation>("Exif::Orientation");
  connect(&thread, SIGNAL(finished()), worker, SLOT(deleteLater()));
  connect(this, SIGNAL(forwardFindImage(quint64, QString, QString, QString,
                                        Exif::Orientation, int, QSize)),
          worker, SLOT(findImage(quint64, QString, QString, QString,
                                 Exif::Orientation, int, QSize)));
  connect(worker, SIGNAL(foundImage(quint64, QImage, bool)),
          this, SLOT(handleFoundImage(quint64, QImage, bool)));
  connect(worker, SIGNAL(exception(QString)),
	  this, SIGNAL(exception(QString)));
  thread.start();
}

ImageFinder::~ImageFinder() {
  thread.quit();
  thread.wait();
}

void ImageFinder::findImage(quint64 id, QString path, QString mods, QString ext,
                            Exif::Orientation orient, int maxdim, QSize ns) {
  queuelength++;
  qDebug() << "ImageFinder::findImage" << id << maxdim << ns;
  emit forwardFindImage(id, path, mods, ext, orient, maxdim, ns);
}

void ImageFinder::handleFoundImage(quint64 id, QImage img, bool fs) {
  queuelength--;
  qDebug() << "ImageFinder::foundImage" << id << img.size() << fs;
  emit foundImage(id, img, fs);
}
