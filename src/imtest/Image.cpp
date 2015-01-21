// Image.cpp

#include "Image.h"
#include "ImageSpaceConverters.h"

using namespace ImageSpaceConverters;

#include <QVariant>

Image::Image(): f(Format::Color8), s(Space::sRGB) {
}

Image::Image(Image const &image): d(image.d), f(image.f), s(image.s) {
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

Image Image::fromImage(QImage const &image) {
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
    return fromImage(image.convertToFormat(QImage::Format_RGB32));
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
    int N = width()*height();
    uchar *dst = img.bits();
    short const *src = (short const *)bits();
    while (N--) {
      short R = src[0]/128;
      dst[2] = R<0 ? 0 : R>255 ? 255 : R;
      short G = src[1]/128;
      dst[1] = G<0 ? 0 : G>255 ? 255 : G;
      short B = src[2]/128;
      dst[0] = B<0 ? 0 : B>255 ? 255 : B;
      dst += 4;
      src += 3;
    }
    return img;
  } else {
    Image img(width(), height(), Format::Color14);
    int N = width()*height();
    short *dst = (short*)img.bits();
    uchar const *src = bits();
    while (N--) {
      dst[2] = src[0] * 128;
      dst[1] = src[1] * 128;
      dst[0] = src[2] * 128;
      dst += 3;
      src += 4;
    }
  }
}


void Image::inPlaceConvertToSpace(Space space) {
  if (space==s || f!=Format::Color14)
    return;
  
  short *dst = (short *)bits();
  int N = byteCount()/6;

  Space sp = s;

  if (sp==Space::sRGB) {
    sRGBToLinear(dst, dst, N);
    sp = Space::LinearRGB;
  } else if (s==Space::LabD50) {
    labD50ToXYZ(dst, dst, N);
    sp = Space::LinearRGB;
  }
  
  // Now we have an image in either LinearRGB or RGB
  if (sp==space)
    return;
  
  if (sp==Space::LinearRGB) 
    linearRGBToXYZ(dst, dst, N);
  else 
    xyzToLinearRGB(dst, dst, N);

  if (space==Space::LabD50) 
    xyzToLabD50(dst, dst, N);
  else if (space==Space::sRGB)
    linearToSRGB(dst, dst, N);
}

Image Image::convertedToSpace(Space space) const {
  if (space==s || f!=Format::Color14)
    return *this;
  
  Image ret(width(), height(), format(), space);
  short *dst = (short *)ret.bits();
  short const *src = (short const *)bits();
  int N = byteCount()/6;

  Space sp = s;

  if (sp==Space::sRGB) {
    sRGBToLinear(dst, src, N);
    src = dst;
    sp = Space::LinearRGB;
  } else if (s==Space::LabD50) {
    labD50ToXYZ(dst, src, N);
    src = dst;
    sp = Space::LinearRGB;
  }
  
  // Now we have an image in either LinearRGB or RGB
  if (sp==space)
    return ret;
  
  if (sp==Space::LinearRGB) 
    linearRGBToXYZ(dst, src, N);
  else 
    xyzToLinearRGB(dst, src, N);

  if (space==Space::LabD50) 
    xyzToLabD50(dst, dst, N);
  else if (space==Space::sRGB)
    linearToSRGB(dst, dst, N);

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
bool Image::load(QString const &filename, char const *format) {
  return false;
}

bool Image::load(class QIODevice *device, char const *format) {
  return false;
}

bool Image::loadFromData(uchar const *data, int length, char const *format) {
  return false;
}

bool Image::loadFromData(QByteArray const &data, char const *format) {
  return false;
}

Image Image::fromData(uchar const *data, int size, char const *format) {
  return Image();
}

Image Image::fromData(QByteArray const &data, char const *format) {
  return Image();
}

bool Image::save(QString const &fileName,
                 char const *format, int quality) const {
  return false;
}

bool Image::save(QIODevice *device,
                 char const *format, int quality) const {
  return false;
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

