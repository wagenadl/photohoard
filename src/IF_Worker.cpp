// IF_Worker.cpp

#include "IF_Worker.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QProcess>

IF_Worker::IF_Worker(QObject *parent): QObject(parent) {
  setObjectName("IF_Worker");
}

void IF_Worker::findImage(quint64 id, QString path, int ver, QString ext,
                          Exif::Orientation orient, int maxdim, QSize ns) {
  try {
    if (ver!=0) {
      emit foundImage(id, QImage());
      return;
    }
  
    if (ext=="jpeg" || ext=="png" || ext=="tiff") {
      // Can do
      QImage img(path);
      if (img.isNull()) {
	emit foundImage(id, QImage());
	return;
      }
    
      if (maxdim>0) 
	if (img.width()>maxdim || img.height()>maxdim)
	  img = img.scaled(QSize(maxdim, maxdim), Qt::KeepAspectRatio);

      switch (orient) {
      case Exif::Upright:
	break;
      case Exif::UpsideDown:
	upsideDown(img);
	break;
      case Exif::CW:
	img = rotateCW(img);
	break;
      case Exif::CCW:
	img = rotateCCW(img);
	break;
      }
      emit foundImage(id, img);
    } else if (ext=="nef" || ext=="cr2") {
      QProcess dcraw;
      QStringList args;
      if (maxdim*2<=ns.width() || maxdim*2<=ns.height())
        args << "-h";
      // If the image is large enough, we might be able to do a quicker
      // load of a downscaled version
      args << "-c";
      args << path;
      dcraw.start("dcraw", args);
      if (!dcraw.waitForStarted())
	throw QString("Could not start DCRaw");
      if (!dcraw.waitForFinished())
	throw QString("Could not complete DCRaw");
      QImage img;
      if (!img.loadFromData(dcraw.readAllStandardOutput()))
	throw QString("Could not parse DCRaw output");
      // change orientation if needed
      emit foundImage(id, img);
    } else {
      // Unknown format
      emit foundImage(id, QImage());
    }
  } catch (QSqlQuery const &q) {
    emit exception("IF_Worker: SqlError: " + q.lastError().text()
		   + " from " + q.lastQuery());
  } catch (QString const &s) {
    emit exception("IF_Worker: " + s);
  } catch (...) {
    emit exception("IF_Worker: Unknown exception");
  }
}

void IF_Worker::upsideDown(QImage &img) {
  int N = img.bytesPerLine()*img.height();
  if (img.format()==QImage::Format_Indexed8) {
    unsigned char *bits = img.bits();
    unsigned char *end = bits + N;
    end -= img.bytesPerLine()-img.width();
    int n = N/2;
    while (--n>=0) {
      uchar b = *bits;
      *bits++ = *--end;
      *end = b;
    }
  } else {
    quint64 *bits = (quint64*)img.bits();
    quint64 *end = bits + N/4;
    end -= img.bytesPerLine() - 4*img.width();
    int n = N/8;
    while (--n>=0) {
      quint64 b = *bits;
      *bits++ = *--end;
      *end = b;
    }
  }
}


QImage IF_Worker::rotateCW(QImage const &img) {
  int W = img.height();
  int H = img.width();
  QImage im2(QSize(W, H), img.format());
  if (img.format()==QImage::Format_Indexed8) {
    unsigned char const *src = img.bits();
    unsigned char *dst = im2.bits();
    for (int y=0; y<H; y++) 
      for (int x=0; x<W; x++) 
        *dst++ = src[(H-1-y) + H*x];
  } else {
    quint64 const *src = (quint64 const *)img.bits();
    quint64 *dst = (quint64 *)im2.bits();
    for (int y=0; y<H; y++) 
      for (int x=0; x<W; x++) 
        *dst++ = src[(H-1-y) + H*x];
  }
  return im2;
}

QImage IF_Worker::rotateCCW(QImage const &img) {
  int W = img.height();
  int H = img.width();
  QImage im2(QSize(W, H), img.format());
  if (img.format()==QImage::Format_Indexed8) {
    unsigned char const *src = img.bits();
    unsigned char *dst = im2.bits();
    for (int y=0; y<H; y++) 
      for (int x=0; x<W; x++) 
        *dst++ = src[y + H*(W-1-x)];
  } else {
    quint64 const *src = (quint64 const *)img.bits();
    quint64 *dst = (quint64 *)im2.bits();
    for (int y=0; y<H; y++) 
      for (int x=0; x<W; x++) 
        *dst++ = src[y + H*(W-1-x)];
  }
  return im2;
}
