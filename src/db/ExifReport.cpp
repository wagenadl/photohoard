// ExifReport.cpp

#include "ExifReport.h"
#include "Exif.h"
#include <QDebug>

void exifreport(QString fn) {
  Exif exif(fn);
  qDebug() << "  " << exif.width() << "x" << exif.height();
  qDebug() << "  " << exif.camera();
  qDebug() << "  " << exif.lens();
  qDebug() << "  " << exif.dateTime();
  qDebug() << "  orient " << exif.orientation();
  qDebug() << "  t = " << exif.exposureTime_s() << " s";
  qDebug() << "  f = 1/" << exif.fNumber();
  qDebug() << "  ISO " << exif.iso();
  qDebug() << "  F = " << exif.focalLength_mm() << " mm";
  qDebug() << "  d = " << exif.focusDistance_m() << " m";
  qDebug() << "  Previews: " << exif.previewSizes().size();
  qDebug() << "";
}
  
