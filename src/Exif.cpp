// Exif.cpp

#include "Exif.h"
#include <QStringList>

Exif::Exif(QString filename):
  image(Exiv2::ImageFactory::open(filename.toUtf8().data())),
  nullDatum(Exiv2::ExifKey("Exif.Photo.ExifVersion")) {
  if (image.get())
    image->readMetadata();
}

bool Exif::ok() const {
  return image->good();
}

Exiv2::Exifdatum const &Exif::exifDatum(QString const &key) const {
  Exiv2::ExifData &data = image->exifData();
  Exiv2::ExifKey k(key.toUtf8().data());
  auto it(data.findKey(k));
  return it==data.end() ? nullDatum : *it;
}
  
int Exif::width() const {
  Rotation r = rotation();
  if (r==Rotation::Upright || r==Rotation::UpsideDown)
    return exifDatum("Exif.Photo.PixelXDimension").toLong();
  else
    return exifDatum("Exif.Photo.PixelYDimension").toLong();
}

int Exif::height() const {
  Rotation r = rotation();
  if (r==Rotation::Upright || r==Rotation::UpsideDown)
    return exifDatum("Exif.Photo.PixelYDimension").toLong();
  else
    return exifDatum("Exif.Photo.PixelXDimension").toLong();
}

Exif::Rotation Exif::rotation() const {
  int rot = exifDatum("Exif.Image.Orientation").toLong();
  switch (rot) {
  case 1: return Rotation::Upright;
  case 3: return Rotation::UpsideDown;
  case 5: return Rotation::CW;
  case 7: return Rotation::CCW;
  default: return Rotation::Upright;
  }
}
    
QString Exif::camera() const {
  QString c = exifDatum("Exif.Image.Model").toString().c_str();
  c.replace("NIKON", "Nikon");
  return c;
}

QString Exif::lens() const {
  QStringList src;
  src << "Exif.NikonLd2.LensIDNumber"
      << "Exif.NikonLd2.LensFStops"
      << "Exif.NikonLd2.MinFocalLength"
      << "Exif.NikonLd2.MaxFocalLength"
      << "Exif.NikonLd2.MaxApertureAtMinFocal"
      << "Exif.NikonLd2.MaxApertureAtMaxFocal"
      << "Exif.NikonLd2.MCUVersion"
      << "Exif.NikonLd2.EffectiveMaxAperture"
      << "Exif.Nikon3.LensType";
  quint64 lensid;
  unsigned char *lensid_ = (unsigned char *)&lensid;
  for (int i=0; i<8; i++)
    lensid_[i] = exifDatum(src[i]).toLong();
  return QString::number(lensid, 16); // really should look up in database
}

double Exif::focalLength_mm() const {
  return exifDatum("Exif.Photo.FocalLength").toFloat();
}
  
double Exif::focusDistance_mm() const {
  Exiv2::Exifdatum const &d(exifDatum("Exif.NikonLd2.FocusDistance"));
  if (d.count()==1) 
    return 5*pow(2,d.toLong()/24.0);
  else
    return 0;
}

double Exif::exposureTime_s() const {
  Exiv2::Exifdatum const &d(exifDatum("Exif.Photo.ExposureTime"));
  if (d.count()==1)
    return d.toFloat();
  else
    return 0;
}
			    
double Exif::fNumber() const {
  Exiv2::Exifdatum const &d(exifDatum("Exif.Photo.FNumber"));
  if (d.count()==1)
    return d.toFloat();
  else
    return 0;
}
  
double Exif::iso() const {
  Exiv2::Exifdatum const &d(exifDatum("Exif.Photo.ISOSpeedRatings"));
  if (d.count()==1)
    return d.toFloat();
  Exiv2::Exifdatum const &d1(exifDatum("Exif.Nikon3.ISOSpeed"));
  if (d1.count()==1)
    return d1.toFloat();
  return 0;
}

QDateTime Exif::dateTime() const {
  Exiv2::Exifdatum const &d(exifDatum("Exif.Photo.DateTimeOriginal"));
  if (d.count()>0) {
    QString dd = d.toString().c_str();
    return QDateTime::fromString("YYYY:MM:DD hh:mm:ss");
  } else {
    return QDateTime();
  }
}

QList<QSize> Exif::previewSizes() const {
  return QList<QSize>();
}

QImage Exif::previewImage(QSize const &) const {
  return QImage();
}

