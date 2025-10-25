// Exif.h

#ifndef EXIF_H

#define EXIF_H

#include <QString>
#include <QDateTime>
#include "PSize.h"
#include <QList>
#include "Image16.h"
#include <exiv2/exiv2.hpp>

class Exif {
#if (EXIV2_MAJOR_VERSION == 0) && (EXIV2_MINOR_VERSION < 28)
using ExivImagePtr = Exiv2::Image::AutoPtr;
#else
using ExivImagePtr = Exiv2::Image::UniquePtr;
#endif
public:
  enum Orientation {
    Upright,
    CCW,
    UpsideDown,
    CW
  };
public:
  static void initialize();
  Exif(QString filename);
  bool ok() const;
  int width() const; // after orientation corrected
  int height() const;
  Orientation orientation() const;
  QString model() const;
  QString make() const;
  QString lens() const;
  double focalLength_mm() const;
  double focusDistance_m() const;
  double exposureTime_s() const;
  double fNumber() const;
  double iso() const;
  QDateTime dateTime() const;
  QList<PSize> previewSizes() const;
  Image16 previewImage(PSize const &) const;
  static bool isRotated(Orientation);
  static PSize fixOrientation(PSize, Orientation);
private:
  Exiv2::Exifdatum const &exifDatum(QString const &) const;
  int64_t intDatum(QString const &key) const;
  int64_t intDatum(Exiv2::Exifdatum const &dat) const;
  int64_t intDatum(Exiv2::Exifdatum const &dat, int idx) const;
  static class NikonLenses const &nikonLenses();
  static class CanonLenses const &canonLenses();
private:
  ExivImagePtr image;
};

#endif
