// IF_Worker.cpp

#include "IF_Worker.h"
#include <QSqlQuery>
#include <QSqlError>
#include "NiceProcess.h"
#include <QDebug>
#include "Adjuster.h"
#include "PSize.h"

IF_Worker::IF_Worker(QObject *parent): QObject(parent) {
  setObjectName("IF_Worker");
  adjuster = new Adjuster(this);
}

Image16 IF_Worker::findImageNow(QString path, QString ext,
				Exif::Orientation orient, QSize naturalSize,
				QString mods,				
				int maxdim, bool urgent,
				QSize *fullSizeReturn) {
  if (fullSizeReturn)
    *fullSizeReturn = QSize();
  bool halfsize = false;
  Image16 img;
  if (ext=="jpeg" || ext=="png" || ext=="tiff") {
    img = Image16(path); // load it directly
  } else if (ext=="nef" || ext=="cr2") {
    NiceProcess dcraw;
    if (!urgent)
      dcraw.renice(10);
    QStringList args;
    if (maxdim>0 && PSize(naturalSize).exceeds(PSize(2*maxdim,2*maxdim))) {
      args << "-h";
      halfsize = true;
    }
    // If the image is large enough, we might be able to do a quicker
    // load of a downscaled version
    args << "-c";
    args << path;
    dcraw.start("dcraw", args);
    if (!dcraw.waitForStarted())
      throw QString("Could not start DCRaw:" + path);
    if (!dcraw.waitForFinished(300000))
      throw QString("Could not complete DCRaw:" + path);
    img = QImage::fromData(dcraw.readAllStandardOutput());
    if (img.isNull())
      throw QString("Could not parse DCRaw output:" + path);
  } else {
    // Other formats?
  }            

  if (img.isNull()) 
    return Image16();

  QSize fullsize = halfsize ? 2*img.size() : img.size();
  if (fullSizeReturn)
    *fullSizeReturn = fullsize;
  
  // This should be reconsidered based on the adjuster
  if (PSize(img.size()).exceeds(PSize(maxdim, maxdim)))
    img = img.scaled(PSize(maxdim, maxdim), Qt::KeepAspectRatio);
    
  switch (orient) {
  case Exif::Upright:
    break;
  case Exif::UpsideDown:
    img.rotate180();
    break;
  case Exif::CW:
    img.rotate90CW();
    break;
  case Exif::CCW:
    img.rotate90CCW();
    break;
  }

  if (mods != "") {
    if (img.size()==fullsize)
      adjuster->setOriginal(img);
    else
      adjuster->setReduced(img, fullsize);
    
    Sliders s(mods);
    img = adjuster->retrieveReduced(s, QSize(maxdim, maxdim));
  }
  
  return img;
}  

void IF_Worker::findImage(quint64 id, QString path, QString ext,
                          Exif::Orientation orient, QSize ns,
			  QString mods,
			  int maxdim, bool urgent) {
  Q_ASSERT(maxdim>0);
  try {
    QSize fullSize;
    Image16 img = findImageNow(path, ext, orient, ns, mods,
			       maxdim, urgent,
			       &fullSize);
    emit foundImage(id, img, fullSize);
  } catch (QSqlQuery const &q) {
    emit exception("IF_Worker: SqlError: " + q.lastError().text()
		   + " from " + q.lastQuery());
  } catch (QString const &s) {
    emit exception("IF_Worker: " + s);
  } catch (...) {
    emit exception("IF_Worker: Unknown exception");
  }
}

