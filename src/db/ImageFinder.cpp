// ImageFinder.cpp

#include "ImageFinder.h"
#include "IF_Worker.h"
#include <QMetaType>
#include "PDebug.h"

ImageFinder::ImageFinder(QObject *parent): QObject(parent) {
  pDebug() << "ImageFinder" << this;
  setObjectName("ImageFinder");
  queuelength = 0;
  worker = new IF_Worker(0);
  worker->moveToThread(&thread);
  qRegisterMetaType<Exif::Orientation>("Exif::Orientation");
  connect(&thread, SIGNAL(finished()), worker, SLOT(deleteLater()));
  connect(this, SIGNAL(forwardFindImage(quint64, QString, QString,
                                        Exif::Orientation, QSize,
					AllAdjustments, int, bool)),
          worker, SLOT(findImage(quint64, QString, QString,
				 Exif::Orientation, QSize,
				 AllAdjustments, int, bool)));
  connect(worker, SIGNAL(foundImage(quint64, Image16, QSize)),
          this, SLOT(handleFoundImage(quint64, Image16, QSize)));
  thread.start();
}

ImageFinder::~ImageFinder() {
  pDebug() << "~ImageFinder" << this;
  thread.quit();
  thread.wait();
}

void ImageFinder::findImage(quint64 id, QString path, QString ext,
			    Exif::Orientation orient, QSize ns,
			    AllAdjustments const &mods,
			    int maxdim, bool urgent) {
  queuelength++;
  pDebug() << "findImage" << this << id << path;
  emit forwardFindImage(id, path, ext, orient, ns, mods, maxdim, urgent);
}

void ImageFinder::handleFoundImage(quint64 id, Image16 img, QSize fs) {
  queuelength--;
  emit foundImage(id, img, fs);
}
