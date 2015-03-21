// ExifReport.cpp

#include "ExifReport.h"
#include "Exif.h"
#include "PDebug.h"

void exifreport(QString fn) {
  Exif exif(fn);
  pDebug() << "  " << exif.width() << "x" << exif.height();
  pDebug() << "  " << exif.make() << exif.model();
  pDebug() << "  " << exif.lens();
  pDebug() << "  " << exif.dateTime();
  pDebug() << "  orient " << exif.orientation();
  pDebug() << "  t = " << exif.exposureTime_s() << " s";
  pDebug() << "  f = 1/" << exif.fNumber();
  pDebug() << "  ISO " << exif.iso();
  pDebug() << "  F = " << exif.focalLength_mm() << " mm";
  pDebug() << "  d = " << exif.focusDistance_m() << " m";
  pDebug() << "  Previews: " << exif.previewSizes().size();
  pDebug() << "";
}
  
