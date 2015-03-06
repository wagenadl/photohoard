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
  connect(this, SIGNAL(forwardFindImage(quint64, QString, QString,
                                        Exif::Orientation, PSize,
					QString, int, bool)),
          worker, SLOT(findImage(quint64, QString, QString,
				 Exif::Orientation, PSize,
				 QString, int, bool)));
  connect(worker, SIGNAL(foundImage(quint64, Image16, PSize)),
          this, SLOT(handleFoundImage(quint64, Image16, PSize)));
  connect(worker, SIGNAL(exception(QString)),
	  this, SIGNAL(exception(QString)));
  thread.start();
}

ImageFinder::~ImageFinder() {
  thread.quit();
  thread.wait();
}

void ImageFinder::findImage(quint64 id, QString path, QString ext,
			    Exif::Orientation orient, PSize ns,
			    QString mods,
			    int maxdim, bool urgent) {
  queuelength++;
  emit forwardFindImage(id, path, ext, orient, ns, mods, maxdim, urgent);
}

void ImageFinder::handleFoundImage(quint64 id, Image16 img, PSize fs) {
  queuelength--;
  emit foundImage(id, img, fs);
}
