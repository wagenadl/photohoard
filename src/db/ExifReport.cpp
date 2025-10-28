// ExifReport.cpp

#include "ExifReport.h"
#include "Exif.h"
#include "PDebug.h"

void exifreport(QString fn) {
  Exif exif(fn);
  qInfo() << "  " << exif.width() << "x" << exif.height();
  qInfo() << "  " << exif.make() << exif.model();
  qInfo() << "  " << exif.lens();
  qInfo() << "  " << exif.dateTime();
  qInfo() << "  orient " << exif.orientation();
  qInfo() << "  t = " << exif.exposureTime_s() << " s";
  qInfo() << "  f = 1/" << exif.fNumber();
  qInfo() << "  ISO " << exif.iso();
  qInfo() << "  F = " << exif.focalLength_mm() << " mm";
  qInfo() << "  d = " << exif.focusDistance_m() << " m";
  qInfo() << "  Previews: " << exif.previewSizes().size();
  qInfo() << "";
}
  
