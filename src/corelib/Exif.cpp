// Exif.cpp

#include "Exif.h"
#include <QStringList>
#include "NikonLenses.h"
#include "CanonLenses.h"
#include <exiv2/preview.hpp>
#include <QByteArray>
#include <QBuffer>
#include <QImageReader>
#include "PDebug.h"
#include <math.h>

void exifLogHandler(int level, char const *message) {
  pDebug() << "Exif msg (" << level << "): " << message;
}

void Exif::initialize() {
  Exiv2::LogMsg::setHandler(exifLogHandler);
}

class ExifInit {
public:
  ExifInit() {
    Exif::initialize();
  }
};

static ExifInit exitInit;

NikonLenses const &Exif::nikonLenses() {
  static NikonLenses lenses;
  return lenses;
}

CanonLenses const &Exif::canonLenses() {
  static CanonLenses lenses;
  return lenses;
}

Exif::Exif(QString filename):
  image(Exiv2::ImageFactory::open(filename.toUtf8().data())),
  nullDatum(Exiv2::ExifKey("Exif.Photo.ExifVersion")) {
  if (image.get()) {
    try {
      image->readMetadata();
    } catch (...) {
      image = Exiv2::Image::AutoPtr();
    }
  }
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
  Orientation r = orientation();
  if (r==Upright || r==UpsideDown)
    return exifDatum("Exif.Photo.PixelXDimension").toLong();
  else
    return exifDatum("Exif.Photo.PixelYDimension").toLong();
}

int Exif::height() const {
  Orientation r = orientation();
  if (r==Upright || r==UpsideDown)
    return exifDatum("Exif.Photo.PixelYDimension").toLong();
  else
    return exifDatum("Exif.Photo.PixelXDimension").toLong();
}

Exif::Orientation Exif::orientation() const {
  int rot = exifDatum("Exif.Image.Orientation").toLong();
  switch (rot) {
  case 1: case 2: return Upright;
  case 3: case 4: return UpsideDown;
  case 8: case 5: return CW;
  case 6: case 7: return CCW;
  default: return Upright; // this should not happen
  }
}

static QString dropWords(QString s, QString pool) {
  QStringList ss = s.split(" ");
  QList<QString> poolbits = pool.split(" ");
  QSet<QString> pp = QSet<QString>(poolbits.begin(), poolbits.end());
  QStringList out;
  for (auto s: ss)
    if (!pp.contains(s))
      out << s;
  return out.join(" ");
}

static QString titleCase(QString s) {
  QStringList ss = s.split(" ");
  QStringList out;
  for (auto s: ss) 
    if (s.size()>=4 && QRegExp("[A-Z]*").exactMatch(s))
      out << s.left(1) + s.mid(1).toLower();
    else
      out << s;
  return out.join(" ");
}

QString Exif::model() const {
  QString c = exifDatum("Exif.Image.Model").toString().c_str();
  return dropWords(titleCase(c.trimmed()), make());
}

QString Exif::make() const {
  QString c = exifDatum("Exif.Image.Make").toString().c_str();
  return dropWords(titleCase(c.trimmed()),
                   "Co., Ltd. Eastman Company Corporation Computer "
                   "CO.,LTD Imaging CORP.");
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
      << "Exif.Nikon3.LensType";
  quint64 lensid;
  unsigned char *lensid_ = (unsigned char *)&lensid;
  for (int i=0; i<8; i++)
    lensid_[i] = exifDatum(src[7-i]).toLong();
  if (nikonLenses().contains(lensid)) 
    return nikonLenses()[lensid];

  src.clear();
  src << "Exif.NikonLd3.LensIDNumber"
      << "Exif.NikonLd3.LensFStops"
      << "Exif.NikonLd3.MinFocalLength"
      << "Exif.NikonLd3.MaxFocalLength"
      << "Exif.NikonLd3.MaxApertureAtMinFocal"
      << "Exif.NikonLd3.MaxApertureAtMaxFocal"
      << "Exif.NikonLd3.MCUVersion"
      << "Exif.Nikon3.LensType";
  for (int i=0; i<8; i++)
    lensid_[i] = exifDatum(src[7-i]).toLong();
  if (nikonLenses().contains(lensid)) 
    return nikonLenses()[lensid];

  Exiv2::Exifdatum const &d(exifDatum("Exif.CanonCs.Lens"));
  if (d.count()>=2) {
    int f_low = d.toLong(1);
    int f_high = d.toLong(0);
    int f_div = d.toLong(2);
    Exiv2::Exifdatum const &dd(exifDatum("Exif.CanonCs.LensType"));
    int typ = (dd.count()>=1) ? dd.toLong(0) : -1;
    if (typ&0x8000) {
      // I'm not going to report on fixed lenses, so let's see
      QString c = exifDatum("Exif.Image.Model").toString().c_str();
      if (!c.contains("EOS"))
        return QString(); // assume fixed lens, boring
    }
    quint64 id = CanonLenses::buildID(typ, f_low, f_high, f_div);
    if (canonLenses().contains(id))
      return canonLenses()[id];
  }

  return QString();
}

double Exif::focalLength_mm() const {
  return exifDatum("Exif.Photo.FocalLength").toFloat();
}
  
double Exif::focusDistance_m() const {
  Exiv2::Exifdatum const &d(exifDatum("Exif.NikonLd2.FocusDistance"));
  if (d.count()==1) 
    return 0.01*pow(10,d.toLong()/40.0);

  Exiv2::Exifdatum const &d2(exifDatum("Exif.NikonLd3.FocusDistance"));
  if (d2.count()==1) 
    return 0.01*pow(10,d2.toLong()/40.0);

  Exiv2::Exifdatum const &d3(exifDatum("Exif.CanonSi.SubjectDistance"));
  if (d3.count()==1) 
    return d3.toLong()/1000; // I don't know if this is at all correct
  
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
    return QDateTime::fromString(dd, "yyyy:MM:dd hh:mm:ss");
  } else {
    return QDateTime();
  }
}

QList<PSize> Exif::previewSizes() const {
  Exiv2::PreviewManager pm(*image);
  Exiv2::PreviewPropertiesList lst(pm.getPreviewProperties());
  QList<PSize> res;
  for (auto i: lst) 
    res << PSize(i.width_, i.height_);
  return res;
}

Image16 Exif::previewImage(PSize const &s0) const {
  Exiv2::PreviewManager pm(*image);
  Exiv2::PreviewPropertiesList lst(pm.getPreviewProperties());
  for (auto i: lst) {
    PSize s(i.width_, i.height_);
    if (s==s0) {
      Exiv2::PreviewImage img(pm.getPreviewImage(i));
      QByteArray ba((char const *)img.pData(), img.size());
      QBuffer buf(&ba);
      QImageReader rdr(&buf);
      return Image16(rdr.read());
    }
  }
  return Image16();
}

bool Exif::isRotated(Exif::Orientation o) {
  return o==Orientation::CW || o==Orientation::CCW;
}

PSize Exif::fixOrientation(PSize s, Exif::Orientation o) {
  return isRotated(o) ? PSize(s.height(), s.width()) : s;
}
