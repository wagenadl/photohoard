// Image.cpp

#include "Image.h"
#include "ImageConverters.h"
#include "ImageSpaceConverters.h"
using namespace ImageSpaceConverters;

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
  f(format), s(space) {
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
  
Image &Image::operator=(Image const &image) {
  d = image.d;
  f = image.f;
  s = image.s;
  return *this;
}

QImage Image::toQImage() const {
  switch (f) {
  case Format::Gray8:
    return convertedToSpace(Space::sRGB).d;
  case Format::Gray16:
    return convertedToSpace(Space::sRGB).convertedToFormat(Format::Gray8).d;
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
    ret.s = Space::sRGB;
    break;
  case QImage::Format_Mono: case QImage::Format_MonoLSB:
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
  if (space==s)
    return *this;
  
  switch (f) {
  case Format::Gray8: 
    if (space==Space::sRGB) {
      Image out(width(), height(), format(), space);
      linearToSRGB(out.bits(), bits(), byteCount());
      return out;
    } else if (s==Space::sRGB) {
      Image out(width(), height(), format(), space);
      sRGBToLinear(out.bits(), bits(), byteCount());
      return out;
    } else {
      Image out(*this);
      out.s = space;
      return out;
    }

  case Format::Color8: {
    Image *iptr=0;
    Space sp = s;
    if (sp==Space::sRGB) {
      iptr = new Image(width(), height(), format(), sp=Space::LinearRGB);
      sRGBToLinear(iptr->bits(), bits(), byteCount());
    } else if (s==Space::LabD50) {
      iptr = new Image(width(), height(), format(), sp=Space::XYZ);
      labD50ToXYZ(iptr->bits(), bits(), byteCount()/4, 4, 255);
    }
    // Now we have an image in either LinearRGB or RGB
    if (sp==space) {
      Image out(*iptr);
      delete iptr;
      return out;
    }
    if (sp==Space::LinearRGB) {
      if (iptr) {
        linearRGBToXYZ(iptr->bits(), iptr->bits(), byteCount()/4, 4);
      } else {
        iptr = new Image(width(), height(), format(), sp=Space::XYZ);
        linearRGBToXYZ(iptr->bits(), bits(), byteCount()/4, 4);
      }
    } else {
      if (iptr) {
        xyzTolinearRGB(iptr->bits(), iptr->bits(), byteCount()/4, 4);
      } else {
        iptr = new Image(width(), height(), format(), sp=Space::LinearRGB);
        xyzTolinearRGB(iptr->bits(), bits(), byteCount()/4, 4);
      }
    }
    if (space==Space::LabD50) 
      xyzToLabD50(iptr->bits(), iptr->bits(), byteCount()/4, 4);
    else if (space==Space::sRGB)
      linearToSRGB(iptr->bits(), iptr->bits(), byteCount()/4, 4);

    Image out(*iptr);
    delete iptr;
    return out;
  }
  case Format::Gray16: {
    if (space==Space::sRGB) {
      Image out(width(), height(), format(), space);
      linearToSRGB((ushort *)out.bits(), (ushort const *)bits(), byteCount()/2);
      return out;
    } else if (s==Space::sRGB) {
      Image out(width(), height(), format(), space);
      sRGBToLinear((ushort *)out.bits(), (ushort const *)bits(), byteCount()/2);
      return out;
    } else {
      Image out(*this);
      out.s = space;
      return out;
    }
  }
    
  case Format::Color16: {
    Image *iptr=0;
    Space sp = s;
    if (sp==Space::sRGB) {
      iptr = new Image(width(), height(), format(), sp=Space::LinearRGB);
      sRGBToLinear((ushort *)iptr->bits(), (ushort const *)bits(),
                   byteCount()/2);
    } else if (s==Space::LabD50) {
      iptr = new Image(width(), height(), format(), sp=Space::XYZ);
      labD50ToXYZ((ushort *)iptr->bits(), (ushort const *)bits(),
                  byteCount()/6, 3);
    }
    // Now we have an image in either LinearRGB or RGB
    if (sp==space) {
      Image out(*iptr);
      delete iptr;
      return out;
    }
    if (sp==Space::LinearRGB) {
      if (iptr) {
        linearRGBToXYZ((ushort *)iptr->bits(), (ushort const *)iptr->bits(),
                       byteCount()/6, 3);
      } else {
        iptr = new Image(width(), height(), format(), sp=Space::XYZ);
        linearRGBToXYZ((ushort *)iptr->bits(), (ushort const *)bits(),
                       byteCount()/6, 3);
      }
    } else {
      if (iptr) {
        xyzTolinearRGB((ushort *)iptr->bits(), (ushort const *)iptr->bits(),
                       byteCount()/6, 3);
      } else {
        iptr = new Image(width(), height(), format(), sp=Space::LinearRGB);
        xyzTolinearRGB((ushort *)iptr->bits(), (ushort const *)bits(),
                       byteCount()/6, 3);
      }
    }
    if (space==Space::LabD50) 
      xyzToLabD50((ushort *)iptr->bits(), (ushort const *)iptr->bits(),
                  byteCount()/6, 3);
    else if (space==Space::sRGB)
      linearToSRGB((ushort *)iptr->bits(), (ushort const *)iptr->bits(),
                   byteCount()/2);

    Image out(*iptr);
    delete iptr;
    return out;
  }
  }
  return Image(); // not executed
}

Image Image::convertedTo(Format format, Space space) const;
bool Image::inPlaceConvertToSpace(Space space);

Image::operator QVariant() const;

// Comparision
bool Image::operator==(Image const &) const;

bool Image::operator!=(Image const &) const;

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

