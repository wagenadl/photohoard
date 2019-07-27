// IF_Worker.cpp

#include "IF_Worker.h"
#include <QSqlQuery>
#include <QSqlError>
#include "NiceProcess.h"
#include "PDebug.h"
#include "AllAdjuster.h"
#include "PSize.h"
#include "Geometry.h"
#include "ImgAvg.h"

IF_Worker::IF_Worker(QObject *parent): QObject(parent) {
  setObjectName("IF_Worker");
  adjuster = new AllAdjuster(this);
}

Image16 IF_Worker::findImageNow(QString path, QString ext,
				Exif::Orientation orient, PSize naturalSize,
				AllAdjustments const &s,
				int maxdim, bool urgent,
				PSize *fullSizeReturn) {
  if (fullSizeReturn)
    *fullSizeReturn = PSize();
  pDebug() << "IF_Worker::findImageNow" << s;

  PSize needed = Geometry::neededScaledOriginalSize(naturalSize,
						    s.baseAdjustments(),
                                                    PSize(maxdim, maxdim));
  bool halfsize = false;
  Image16 img;
  if (ext=="jpeg" || ext=="png" || ext=="tiff") {
    img = Image16(path); // load it directly
  } else if (ext=="nef" || ext=="cr2") {
    NiceProcess dcraw;
    if (!urgent)
      dcraw.renice(10);
    QStringList args;
    if (maxdim>0 && PSize(naturalSize/2).isLargeEnoughFor(needed)) {
      args << "-h";
      halfsize = true;
    }
    // If the image is large enough, we might be able to do a quicker
    // load of a downscaled version
    args << QString("-c -t 0 -w -4 -o 5").split(" ");
    args << path;
    dcraw.start("dcraw", args);
    if (!dcraw.waitForStarted()) {
      COMPLAIN("Could not start DCRaw:" + path);
      return Image16();
    }
    if (!dcraw.waitForFinished(300000)) {
      COMPLAIN("Could not complete DCRaw:" + path);
      return Image16();
    }
    img = QImage::fromData(dcraw.readAllStandardOutput());
    if (img.isNull()) {
      COMPLAIN("Could not parse DCRaw output:" + path);
      return Image16();
    }
  } else {
    // Other formats?
  }            

  if (img.isNull()) 
    return Image16();

  PSize fullsize = Exif::fixOrientation(halfsize ? 2*img.size() : img.size(),
                                        orient);
  if (fullSizeReturn)
    *fullSizeReturn = fullsize;

  img = img.scaledDownToFitIn(Exif::fixOrientation(needed, orient));

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

  if (!s.isDefault()) {
    if (img.size()==fullsize)
      adjuster->setOriginal(img);
    else
      adjuster->setReduced(img, fullsize);
    
    img = adjuster->retrieveReduced(s, PSize(maxdim, maxdim));
  }
  pDebug() << "IF_Worker: findnow" << img.size() << averagePixel(img);
  return img;
}  

void IF_Worker::findImage(quint64 id, QString path, QString ext,
                          Exif::Orientation orient, QSize ns,
			  AllAdjustments mods,
			  int maxdim, bool urgent) {
  Q_ASSERT(maxdim>0);
  PSize fullSize;
  Image16 img = findImageNow(path, ext, orient, ns, mods,
                             maxdim, urgent,
                             &fullSize);
  emit foundImage(id, img, fullSize);
}

