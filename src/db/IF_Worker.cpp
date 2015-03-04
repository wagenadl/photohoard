// IF_Worker.cpp

#include "IF_Worker.h"
#include <QSqlQuery>
#include <QSqlError>
#include "NiceProcess.h"
#include <QDebug>
#include "Adjuster.h"

IF_Worker::IF_Worker(QObject *parent): QObject(parent) {
  setObjectName("IF_Worker");
  adjuster = new Adjuster(this);
}

Image16 IF_Worker::findImageNow(QString path, QString mods, QString ext,
                               Exif::Orientation orient, int maxdim, QSize ns,
                               bool *fullSizeReturn) {
  bool fullSize = true;
  Image16 img;
  if (ext=="jpeg" || ext=="png" || ext=="tiff") {
    img = Image16(path); // load it directly
  } else if (ext=="nef" || ext=="cr2") {
    NiceProcess dcraw;
    if (maxdim>0)
      dcraw.renice(10);
    QStringList args;
    if (maxdim>0
        && (maxdim*2<=ns.width() || maxdim*2<=ns.height())) {
      args << "-h";
      fullSize = false;
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

  if (img.isNull()) {
    if (fullSizeReturn)
      *fullSizeReturn = false;
    return Image16();
  }
  
  if (maxdim>0) {
    // This should be reconsidered based on the adjuster
    if (img.width()>maxdim || img.height()>maxdim) {
      img = img.scaled(QSize(maxdim, maxdim), Qt::KeepAspectRatio);
      fullSize = false;
    }
  }
    
  switch (orient) {
  case Exif::Upright:
    break;
  case Exif::UpsideDown:
    img = upsideDown(img);
    break;
  case Exif::CW:
    img = rotateCW(img);
    break;
  case Exif::CCW:
    img = rotateCCW(img);
    break;
  }

  if (mods != "") {
    if (fullSize)
      adjuster->setOriginal(img);
    else
      adjuster->setReduced(img, ns);
    Sliders s(mods);
    if (maxdim)
      img = adjuster->retrieveReduced(s, QSize(maxdim, maxdim));
    else
      img = adjuster->retrieveFull(s);
  }
  
  if (fullSizeReturn)
    *fullSizeReturn = fullSize;
  
  return img;
}  

void IF_Worker::findImage(quint64 id, QString path, QString mods, QString ext,
                          Exif::Orientation orient, int maxdim, QSize ns) {
  try {
    bool fullSize;
    Image16 img = findImageNow(path, mods, ext, orient, maxdim, ns,
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

Image16 IF_Worker::upsideDown(Image16 &img) {
  img.rotate180();
  return img;
}


Image16 IF_Worker::rotateCW(Image16 &img) {
  img.rotate90CW();
  return img;
}

Image16 IF_Worker::rotateCCW(Image16 &img) {
  img.rotate90CCW();
  return img;
}
