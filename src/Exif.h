// Exif.h

#ifndef EXIF_H

#define EXIF_H

#include <QString>
#include <QDateTime>
#include <QSize>
#include <QList>
#include <QImage>
#include <exiv2/exiv2.hpp>

class Exif {
public:
  enum Orientation {
    Upright,
    CCW,
    UpsideDown,
    CW
  };
public:
  Exif(QString filename);
  bool ok() const;
  int width() const; // after orientation corrected
  int height() const;
  Orientation orientation() const;
  QString camera() const;
  QString lens() const;
  double focalLength_mm() const;
  double focusDistance_m() const;
  double exposureTime_s() const;
  double fNumber() const;
  double iso() const;
  QDateTime dateTime() const;
  QList<QSize> previewSizes() const;
  QImage previewImage(QSize const &) const;
private:
  Exiv2::Exifdatum const &exifDatum(QString const &) const;
  static class NikonLenses const &nikonLenses();
private:
  Exiv2::Image::AutoPtr image;
  Exiv2::Exifdatum nullDatum;
};

#endif
