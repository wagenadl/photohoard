// Image.cpp

#include "Image.h"
#include "ImageSpaceConverters.h"

using namespace ImageSpaceConverters;

#include <QVariant>
#include <QDebug>

Image::Image(): f(Format::Color8), s(Space::sRGB) {
}

Image::Image(Image const &image): d(image.d), f(image.f), s(image.s) {
}

Image::Image(QSize size, Image::Format format, Image::Space space):
  Image(size.width(), size.height(), format, space) {
}

Image::Image(uint width, uint height, Image::Format format, Image::Space space):
  f(format), s(space) {
  switch (f) {
  case Format::Color8:
    d = QImage(width, height, QImage::Format_RGB32);
    s = Space::sRGB;
    break;
  case Format::Color14:
    d = QImage(width*3, height, QImage::Format_RGB16);
    break;
  }
}

Image::Image(uchar const *data, uint width, uint height,
             Image::Format format, Image::Space space):
  f(format), s(space) {
  switch (f) {
  case Format::Color8:
    d = QImage(data, width, height, QImage::Format_RGB32);
    s = Space::sRGB;
    break;
  case Format::Color14:
    d = QImage(data, width*3, height, QImage::Format_RGB16);
    break;
  }
}    

Image::Image(uchar const *data, uint width, uint height, uint bytesPerLine,
             Image::Format format, Image::Space space): f(format), s(space) {
  switch (f) {
  case Format::Color8:
    d = QImage(data, width, height, bytesPerLine, QImage::Format_RGB32);
    s = Space::sRGB;
    break;
  case Format::Color14:
    d = QImage(data, width*3, height, bytesPerLine, QImage::Format_RGB16);
    break;
  }
}
  
Image &Image::operator=(Image const &image) {
  d = image.d;
  f = image.f;
  s = image.s;
  return *this;
}

QImage Image::toQImage() const {
  switch (f) {
  case Format::Color8:
    return d;
  case Format::Color14:
    return convertedToSpace(Space::sRGB).convertedToFormat(Format::Color8).d;
  }
}

Image Image::fromQImage(QImage const &image) {
  Image ret;
  switch (image.format()) {
  case QImage::Format_Invalid:
    break;
  case QImage::Format_RGB32:
    ret.d = image;
    ret.f = Format::Color8;
    ret.s = Space::sRGB;
    break;
  default:
    return fromQImage(image.convertToFormat(QImage::Format_RGB32));
  }
  return ret;
}

Image Image::convertedToFormat(Format format) const {
  if (format==f)
    return *this;
  
  if (format==Format::Color8) {
    if (s!=Space::sRGB)
      return convertedToSpace(Space::sRGB).convertedToFormat(format);
    
    Image img(width(), height(), Format::Color8);
    int X = width();
    int Y = height();
    int dLs = bytesPerLine()/2 - 3*X;
    int dLd = img.bytesPerLine() - 4*X;
    uchar *dst = img.bits();
    short const *src = (short const *)bits();
    while (Y--) {
      for (int x=0; x<X; x++) {
        short R = src[0]/64;
        dst[2] = R<0 ? 0 : R>255 ? 255 : R;
        short G = src[1]/64;
        dst[1] = G<0 ? 0 : G>255 ? 255 : G;
        short B = src[2]/64;
        dst[0] = B<0 ? 0 : B>255 ? 255 : B;
        dst += 4;
        src += 3;
      }
      src += dLs;
      dst += dLd;
    }
    return img;
  } else {
    Image img(width(), height(), Format::Color14);
    int X = width();
    int Y = height();
    int dLs = bytesPerLine() - 4*X;
    int dLd = img.bytesPerLine()/2 - 3*X;
    short *dst = (short*)img.bits();
    uchar const *src = bits();
    while (Y--) {
      for (int x=0; x<X; x++) {
        dst[2] = src[0] * 64;
        dst[1] = src[1] * 64;
        dst[0] = src[2] * 64;
        dst += 3;
        src += 4;
      }
      src += dLs;
      dst += dLd;
    }
    return img;
  }
}


void Image::inPlaceConvertToSpace(Space space) {
  qDebug() << "inplaceconverttospace" << int(space);
  if (space==s || f!=Format::Color14)
    return;
  
  short *dst = (short *)bits();
  int X = width();
  int Y = height();
  int L = bytesPerLine()/2;

  Space sp = s;
  s = space;

  if (sp==Space::sRGB) {
    sRGBToLinear(dst, dst, X, Y, L);
    sp = Space::LinearRGB;
  } else if (s==Space::LabD50) {
    labD50ToXYZ(dst, dst, X, Y, L);
    sp = Space::LinearRGB;
  }
  
  // Now we have an image in either LinearRGB or RGB
  if (sp==space)
    return;
  
  if (sp==Space::LinearRGB && space!=Space::sRGB) 
    linearRGBToXYZ(dst, dst, X, Y, L);
  else if (sp==Space::XYZ && space!=Space::LabD50)
    xyzToLinearRGB(dst, dst, X, Y, L);

  if (space==Space::LabD50) 
    xyzToLabD50(dst, dst, X, Y, L);
  else if (space==Space::sRGB)
    linearToSRGB(dst, dst, X, Y, L);
}

Image Image::convertedToSpace(Space space) const {
  qDebug() << "convertedtospace" << int(space);
  if (space==s || f!=Format::Color14)
    return *this;
  
  Image ret(width(), height(), format(), space);
  short *dst = (short *)ret.bits();
  short const *src = (short const *)bits();
  int X = width();
  int Y = height();
  int L = bytesPerLine()/2;

  Space sp = s;

  if (sp==Space::sRGB) {
    sRGBToLinear(dst, src, X, Y, L);
    src = dst;
    sp = Space::LinearRGB;
  } else if (s==Space::LabD50) {
    labD50ToXYZ(dst, src, X, Y, L);
    src = dst;
    sp = Space::XYZ;
  }
  
  // Now we have an image in either LinearRGB or RGB
  if (sp==space)
    return ret;
  
  if (sp==Space::LinearRGB && space!=Space::sRGB) {
    linearRGBToXYZ(dst, src, X, Y, L);
    src = dst;
  } else if (sp==Space::XYZ && space!=Space::LabD50) {
    xyzToLinearRGB(dst, src, X, Y, L);
    src = dst;
  }

  if (space==Space::LabD50) 
    xyzToLabD50(dst, src, X, Y, L);
  else if (space==Space::sRGB)
    linearToSRGB(dst, src, X, Y, L);

  return ret;
}

Image Image::convertedTo(Format format, Space space) const {
  Image ret = convertedToFormat(format);
  ret.inPlaceConvertToSpace(space);
  return ret;
}

Image::operator QVariant() const {
  return QVariant();
}

// Comparision
bool Image::operator==(Image const &o) const {
  return d==o.d && s==o.s;
}

bool Image::operator!=(Image const &o) const {
  return d!=o.d || s!=o.s;
}

// Loading and saving

Image::Image(QString const &filename, char const *format) {
  load(filename, format);
}

Image::Image(char const *filename, char const *format) {
  load(filename, format);
}

bool Image::load(QString const &filename, char const *format) {
  QImage img;
  if (img.load(filename, format)) {
    d = img.convertToFormat(QImage::Format_RGB32);
    f = Format::Color8;
    s = Space::sRGB;
    return true;
  } else {
    d = QImage();
    return false;
  }
}

bool Image::load(class QIODevice *device, char const *format) {
  QImage img;
  if (img.load(device, format)) {
    d = img.convertToFormat(QImage::Format_RGB32);
    f = Format::Color8;
    s = Space::sRGB;
    return true;
  } else {
    d = QImage();
    return false;
  }
}

bool Image::loadFromData(uchar const *data, int length, char const *format) {
  QImage img;
  if (img.loadFromData(data, length, format)) {
    d = img.convertToFormat(QImage::Format_RGB32);
    f = Format::Color8;
    s = Space::sRGB;
    return true;
  } else {
    d = QImage();
    return false;
  }
}

bool Image::loadFromData(QByteArray const &data, char const *format) {
  QImage img;
  if (img.loadFromData(data, format)) {
    d = img.convertToFormat(QImage::Format_RGB32);
    f = Format::Color8;
    s = Space::sRGB;
    return true;
  } else {
    d = QImage();
    return false;
  }
}

Image Image::fromData(uchar const *data, int size, char const *format) {
  return fromQImage(QImage::fromData(data, size, format));
}

Image Image::fromData(QByteArray const &data, char const *format) {
  return fromQImage(QImage::fromData(data, format));
}

bool Image::save(QString const &fileName,
                 char const *format, int quality) const {
  QImage img = convertedTo(Format::Color8, Space::sRGB).d;
  return img.save(fileName, format, quality);
}

bool Image::save(QIODevice *device,
                 char const *format, int quality) const {
  QImage img = convertedTo(Format::Color8, Space::sRGB).d;
  return img.save(device, format, quality);
}

// In-place manipulation
void Image::invertPixels(QImage::InvertMode mode) {
}

// Modified copies
Image Image::mirrored(bool horizontal, bool vertical) const {
  return Image();
}

Image Image::copy(QRect const &rect) const {
  return Image();
}

Image Image::copy(int x, int y, int width, int height) const {
  return Image();
}

Image Image::scaled(QSize const &size,
                    Qt::AspectRatioMode aspectRatioMode,
                    TransformationMode transformMode) const {
  return Image();
}

Image Image::scaled(int width, int height,
                    Qt::AspectRatioMode aspectRatioMode,
                    TransformationMode transformMode) const {
  return Image();
}

Image Image::scaledToHeight(int height,
                            TransformationMode transformMode) const {
  return Image();
}

Image Image::scaledToWidth(int width,
                           TransformationMode transformMode) const {
  return Image();
}

Image Image::transformed(QMatrix const &matrix,
                         TransformationMode transformMode) const {
  return Image();
}

Image Image::transformed(QTransform const &matrix,
                         TransformationMode transformMode) const {
  return Image();
}

QMatrix Image::trueMatrix(QMatrix const &matrix, int width, int height) {
  return QImage::trueMatrix(matrix, width, height);
}

QTransform Image::trueMatrix(QTransform const &matrix, int width, int height) {
  return QImage::trueMatrix(matrix, width, height);
}

