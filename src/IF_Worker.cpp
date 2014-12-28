// IF_Worker.cpp

#include "IF_Worker.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QProcess>
#include <QDebug>

IF_Worker::IF_Worker(QObject *parent): QObject(parent) {
  setObjectName("IF_Worker");
}

void IF_Worker::findImage(quint64 id, QString path, QString mods, QString ext,
                          Exif::Orientation orient, int maxdim, QSize ns) {
  try {
    if (!mods.isEmpty()) {
      emit foundImage(id, QImage(), false);
      return;
    }

    bool fullSize = true;
    QImage img;
    if (ext=="jpeg" || ext=="png" || ext=="tiff") {
      img = QImage(path); // load it directly
    } else if (ext=="nef" || ext=="cr2") {
      QProcess dcraw;
      QStringList args;
      if (maxdim*2<=ns.width() || maxdim*2<=ns.height()) {
        args << "-h";
        fullSize = false;
      }
      // If the image is large enough, we might be able to do a quicker
      // load of a downscaled version
      args << "-c";
      args << path;
      dcraw.start("dcraw", args);
      if (!dcraw.waitForStarted())
	throw QString("Could not start DCRaw");
      if (!dcraw.waitForFinished())
	throw QString("Could not complete DCRaw");
      if (!img.loadFromData(dcraw.readAllStandardOutput()))
	throw QString("Could not parse DCRaw output");
    } else {
      // Other formats?
    }            

    if (img.isNull()) {
      emit foundImage(id, QImage(), false);
      return;
    }
    if (maxdim>0) {
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

QImage IF_Worker::upsideDown(QImage &img) {
  return img.mirrored(true, true);
}


QImage IF_Worker::rotateCW(QImage &img) {
  int W = img.height();
  int H = img.width();
  int DXsrc = img.bytesPerLine();
  QImage im2(QSize(W, H), img.format());
  switch (img.format()) {
  case QImage::Format_Indexed8: {
    unsigned char const *src = img.bits();
    for (int y=0; y<H; y++) {
      unsigned char *dst = im2.scanLine(y);
      for (int x=0; x<W; x++) 
        *dst++ = src[(H-1-y) + DXsrc*x];
    }
  } break;
  case QImage::Format_RGB32: case QImage::Format_ARGB32: {
    quint32 const *src = (quint32 const *)img.bits();
    DXsrc /= 4;
    for (int y=0; y<H; y++) {
      quint32 *dst = (quint32 *)im2.scanLine(y);
      for (int x=0; x<W; x++) 
        *dst++ = src[(H-1-y) + DXsrc*x];
    }
  } break;
  default:
    throw QString("IF_Worker: rotateCW: Unsupported format: %1")
      .arg(img.format());
    break;
  }
  return im2;
}

QImage IF_Worker::rotateCCW(QImage &img) {
  int W = img.height();
  int H = img.width();
  int DXsrc = img.bytesPerLine();
  QImage im2(QSize(W, H), img.format());
  switch (img.format()) {
  case QImage::Format_Indexed8: {
    unsigned char const *src = img.bits();
    for (int y=0; y<H; y++) {
      unsigned char *dst = im2.scanLine(y);
      for (int x=0; x<W; x++) 
        *dst++ = src[y + DXsrc*(W-1-x)];
    }
  } break;
  case QImage::Format_RGB32: case QImage::Format_ARGB32: {
    quint32 const *src = (quint32 const *)img.bits();
    DXsrc /= 4;
    for (int y=0; y<H; y++) {
      quint32 *dst = (quint32 *)im2.scanLine(y);
      for (int x=0; x<W; x++) 
        *dst++ = src[y + DXsrc*(W-1-x)];
    }
  } break;
  default:
    throw QString("IF_Worker: rotateCCW: Unsupported format: %1")
      .arg(img.format());
    break;
  }
  return im2;
}
