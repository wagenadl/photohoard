// Image.cpp

#include "Image.h"

Image::Image(): f(Format::Gray8), s(Space::sRGB) {
}

Image::Image(Image const &image): d(image.d), f(image.f), s(image.s) {
}

Image::Image(uint width, uint height, Image::Format format, Image::Space space):
  f(format), s(space) {
  switch (f) {
  case Format::Gray8:
    d = QImage(width, height, QImage::Format_Indexed8);
    createGrayLut();
    break;
  case Format::Color8:
    d = QImage(width, height, QImage::Format_RGB32);
    break;
  case Format::Gray16:
    d = QImage(width, height, QImage::Format_RGB16);
    break;
  case Format::Color16:
    d = QImage(width*3, height, QImage::Format_RGB16);
    break;
  }
}

Image::Image(uchar const *data, uint width, uint height,
             Image::Format format, Image::Space space):
  f(format), space(s) {
  switch (f) {
  case Format::Gray8:
    d = QImage(data, width, height, QImage::Format_Indexed8);
    createGrayLut();
    break;
  case Format::Color8:
    d = QImage(data, width, height, QImage::Format_RGB32);
    break;
  case Format::Gray16:
    d = QImage(data, width, height, QImage::Format_RGB16);
    break;
  case Format::Color16:
    d = QImage(data, width*3, height, QImage::Format_RGB16);
    break;
  }
}    

Image::Image(uchar const *data, uint width, uint height, uint bytesPerLine,
             Image::Format format, Image::Space space): f(format), s(space) {
  switch (f) {
  case Format::Gray8:
    d = QImage(data, width, height, bytesPerLine, QImage::Format_Indexed8);
    createGrayLut();
    break;
  case Format::Color8:
    d = QImage(data, width, height, bytesPerLine, QImage::Format_RGB32);
    break;
  case Format::Gray16:
    d = QImage(data, width, height, bytesPerLine, QImage::Format_RGB16);
    break;
  case Format::Color16:
    d = QImage(data, width*3, height, bytesPerLine, QImage::Format_RGB16);
    break;
  }
}
  
Image Image::&operator=(Image const &image) {
  d = image.d;
  f = image.f;
  s = image.s;
  return *this;
}

QImage Image::toQImage() const {
  switch (f) {
  case Format::Gray8: case Format::sRGB8:
    return d;
  case Format::Gray16:
    return convertedToFormat(Format::Gray8).d;
  case Format::Color8:
    return convertedToSpace(Space::sRGB).d;
  case Format::Color16:
    return convertedToSpace(Space::sRGB).convertedToFormat(Format::Color8).d;
  }
}    

Image Image::fromImage(QImage const &image) {
  Image ret;
  switch (image.format()) {
  case QImage::Format_Invalid:
    break;
  case QImage::Format_Indexed8:
    ret.d = image;
    ret.createGrayLut();
    ret.f = Format::Gray8;
    break;
  case QImage::Mono: case QImage::MonoLSB:
    return fromImage(image.convertToFormat(QImage::Format_Indexed8));
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
  switch (format) {
  case Format::Gray8:
    switch (f) {
    case Format::Gray8:
      return *this;
    case Format::Color8:
      return ImageConverters::color8ToGray8(d, s);
    case Format::Gray16:
      return ImageConverters::gray16ToGray8(d, s);
    case Format::Color16:
      return ImageConverters::color16ToGray8(d, s);
    }
  case Format::Color8:
    switch (f) {
    case Format::Gray8:
      return ImageConverters::gray8ToColor8(d, s);
    case Format::Color8:
      return *this;
    case Format::Gray16:
      return ImageConverters::gray8ToGray16(d, s);
    case Format::Color16:
      return ImageConverters::color8ToColor16(d, s);
    }
  case Format::Gray16:
    switch (f) {
    case Format::Gray8:
      return ImageConverters::gray8ToGray16(d, s);
    case Format::Color8:
      return ImageConverters::color8ToGray16(d, s);
    case Format::Gray16:
      return *this;
    case Format::Color16:
      return ImageConverters::color16ToGray16(d, s);
    }
  case Format::Color16:
    switch (f) {
    case Format::Gray8:
      return ImageConverters::gray8ToColor16(d, s);
    case Format::Color8:
      return ImageConverters::color8ToColor16(d, s);
    case Format::Gray16:
      return ImageConverters::gray16ToColor16(d, s);
    case Format::Color16:
      return *this;
    }
  }
  return Image(); // not executed
}

Image Image::convertedToSpace(Space space) const {
  if (f==Format::Gray8 || f==Format::Gray16)
    return *this;
  if (space==s)
    return *this;
  //
}
Image Image::convertedTo(Format format, Space space) const;
bool Image::inPlaceConvertToSpace(Space space);

Image::operator QVariant() const;

// Comparision
bool Image::operator==(Image const &) const;

bool Image::operator!=(Image const &) const;

// Basic information
uint Image::width() const;

uint Image::height() const;

QSize Image::size() const;

uint Image::bytesPerLine() const;

uint Image::byteCount() const;

Format Image::format() const;

bool Image::hasAlphaChannel() const;

bool Image::isGrayscale() const;

bool Image::isNull() const;

// Access functions
uchar const Image::*constBits() const;

uchar const Image::*bits() const;

uchar Image::*bits();

uchar const Image::*constScanLine(uint y) const;

uchar const Image::*scanLine(uint y) const;

uchar Image::*scanLine(uint y);

uchar const Image::*constPixel(uint x, uint y) const;

uchar const Image::*pixel(uint x, uint y) const;

uchar Image::*pixel(uint x, uint y);

uchar const Image::*constPixel(QPoint const &xy) const;

uchar const Image::*pixel(QPoint const &xy) const;

uchar Image::*pixel(QPoint const &xy);

bool Image::valid(uint x, uint y) const;

bool Image::valid(QPoint const &xy) const;

// Loading and saving
bool Image::load(QString const &filename, char const *format=0);

bool Image::load(class QIODevice *device, char const *format=0);

bool Image::loadFromData(uchar const *data, int length, char const *format=0);

bool Image::loadFromData(QByteArray const &data, char const *format=0);

static Image Image::fromData(uchar const *data, int size, char const *format=0);

static Image Image::fromData(QByteArray const &data, char const *format=0);

bool Image::save(QString const &fileName,
                 char const *format=0, int quality=-1) const;

bool Image::save(QIODevice *device,
                 char const *format=0, int quality=-1 ) const;

// In-place manipulation
void Image::invertPixels(QImage::InvertMode mode=QImage::InvertRgb);

// Modified copies
Image Image::mirrored(bool horizontal=false, bool vertical=true) const;

Image Image::copy(QRect const &rectange=QRect()) const;

Image Image::copy(int x, int y, int width, int height) const;

Image Image::scaled(QSize const &size,
                    Qt::AspectRatioMode aspectRatioMode=Qt::IgnoreAspectRatio,
                    TransformationMode transformMode=TransformationMode::Fast)
  const;

Image Image::scaled(int width, int height,
                    Qt::AspectRatioMode aspectRatioMode=Qt::IgnoreAspectRatio,
                    TransformationMode transformMode=TransformationMode::Fast)
  const;

Image Image::scaledToHeight(int height,
                     TransformationMode transformMode=TransformationMode::Fast)
  const;

Image Image::scaledToWidth(int width,
                    TransformationMode transformMode=TransformationMode::Fast)
  const;

Image Image::transformed(QMatrix const &matrix,
                  TransformationMode transformMode=TransformationMode::Fast)
  const;

Image Image::transformed(QTransform const &matrix,
                  TransformationMode transformMode=TransformationMode::Fast)
  const;

QMatrix Image::trueMatrix(QMatrix const &matrix, int width, int height);

QTransform Image::trueMatrix(QTransform const &matrix, int width, int height);

