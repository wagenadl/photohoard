// Image16.cpp

#include "Image16.h"
#include <QDebug>
#include "ColorSpaces.h"

class Image16Foo {
public:
  Image16Foo() {
    qRegisterMetaType<Image16>("Image16");
  }
};

static Image16Foo foo;


Image16::Image16() {
}

Image16::Image16(QString const &fn, char const *format):
  Image16(QImage(fn, format)) {
}

Image16::Image16(char const *fn, char const *format):
  Image16(QString(fn), format) {
}

Image16::Image16(Image16 const &image): d(image.d) {
}

Image16::Image16(int width, int height, Image16::Format format):
  d(new Image16::Data(width, height, format)) {
}

Image16::Image16(QSize size, Image16::Format format):
  Image16(size.width(), size.height(), format) {
}

Image16::Image16(uchar const *data, int width, int height,
                 Image16::Format format) {
  if (format==Format::sRGB8) 
    d = new Data(QImage(data, width, height, QImage::Format_RGB32));
  else
    d = new Data(QImage(data, width*3, height, QImage::Format_RGB16));
  d->width = width;
  d->format = format;
}

Image16::Image16(uchar const *data, int width, int height, int bytesPerLine,
                 Image16::Format format) {
  if (format==Format::sRGB8) 
    d = new Data(QImage(data, width, height, bytesPerLine,
                        QImage::Format_RGB32));
  else
    d = new Data(QImage(data, width*3, height, bytesPerLine,
                        QImage::Format_RGB16));
  d->width = width;
  d->format = format;
}

Image16 &Image16::operator=(Image16 const &image) {
  d = image.d;
  return *this;
}

QImage Image16::toQImage() const {
  if (format()==Format::sRGB8)
    return d->image;
  else
    return convertedTo(Format::sRGB8).toQImage();
}

Image16::Image16(QImage const &image): d(new Data(image)) {
}

Image16 Image16::fromQImage(QImage const &image) {
  return Image16(image);
}

void Image16::convertFrom(Image16 const &other) {
  if (width()==other.width() && height()==other.height()) 
    convertFrom(other, other.format());
  else
    *this = other.convertedTo(format());
}

template <typename SRC> void convertFromTemplate(Image16 *dst, SRC *src,
                                                 int SB) {
  int w = dst->width();
  int h = dst->height();
  uint8_t *d = dst->bytes();
  int db = dst->bytesPerLine();
  switch (dst->format()) {
  case Image16::Format::sRGB8:
    ColorSpaces::convertImage(src, w, h, SB, (ColorSpaces::sRGB *)d, db);
    break;
  case Image16::Format::XYZ16:
    ColorSpaces::convertImage(src, w, h, SB, (ColorSpaces::XYZ *)d, db);
    break;
  case Image16::Format::XYZp16:
    ColorSpaces::convertImage(src, w, h, SB, (ColorSpaces::XYZp *)d, db);
    break;
  case Image16::Format::Lab16:
    ColorSpaces::convertImage(src, w, h, SB, (ColorSpaces::Lab *)d, db);
    break;
  case Image16::Format::LMS16:
    ColorSpaces::convertImage(src, w, h, SB, (ColorSpaces::LMS *)d, db);
    break;
  case Image16::Format::IPT16:
    ColorSpaces::convertImage(src, w, h, SB, (ColorSpaces::IPT *)d, db);
    break;
  }
}

void Image16::convertFrom(Image16 const &other, Image16::Format sfmt) {
  if (format()==sfmt) {
    memcpy(bytes(), other.bytes(), byteCount());
  } else {
    switch (sfmt) {
    case Format::sRGB8:
      convertFromTemplate(this, (ColorSpaces::sRGB const *)other.bytes(),
                          other.bytesPerLine());
      break;
    case Format::XYZ16:
      convertFromTemplate(this, (ColorSpaces::XYZ const *)other.bytes(),
                          other.bytesPerLine());
      break;
    case Format::XYZp16:
      convertFromTemplate(this, (ColorSpaces::XYZp const *)other.bytes(),
                          other.bytesPerLine());
      break;
    case Format::Lab16:
      convertFromTemplate(this, (ColorSpaces::Lab const *)other.bytes(),
                          other.bytesPerLine());
      break;
    case Format::LMS16:
      convertFromTemplate(this, (ColorSpaces::LMS const *)other.bytes(),
                          other.bytesPerLine());
      break;
    case Format::IPT16:
      convertFromTemplate(this, (ColorSpaces::IPT const *)other.bytes(),
                          other.bytesPerLine());
      break;
    }
  }
}

Image16 Image16::convertedTo(Format fmt) const {
  if (format()==fmt)
    return Image16(*this);

  Image16 res(width(), height(), fmt);
  res.convertFrom(*this);
  return res;
}

void Image16::convertTo(Format fmt) {
  if (format()==fmt)
    return;
  if (format()==Format::sRGB8 || fmt==Format::sRGB8) {
    Image16 tmp = convertedTo(fmt);
    *this = tmp;
  } else {
    Format oldfmt = format();
    d->format = fmt;
    convertFrom(*this, oldfmt);
  }
}

Image16 Image16::scaled(QSize s, Qt::AspectRatioMode arm) const {
  return fromQImage(toQImage().scaled(s, arm));
  // This should be smarter
}

Image16 Image16::scaledToWidth(int w, Qt::TransformationMode tm) const {
  return fromQImage(toQImage().scaledToWidth(w, tm));
  // This should be smarter
}

Image16 Image16::scaledToHeight(int h, Qt::TransformationMode tm) const {
  return fromQImage(toQImage().scaledToHeight(h, tm));
  // This should be smarter
}

void Image16::rotate90CW() {
  Image16 dst(QSize(height(), width()), format());
  int bpp = bytesPerPixel();
  int X = dst.width();
  int Y = dst.height();
  int SL = bytesPerLine();
  int DL = dst.bytesPerLine();
  uchar const *s = bytes();
  uchar *d = dst.bytes();
  for (int y=0; y<Y; y++) {
    uchar const *s1 = s + bpp*(Y-1-y);
    uchar *d1 = d + DL*y;
    for (int x=0; x<X; x++) {
      uchar const *s2 = s1 + SL*(X-1-x);
      memcpy(d1, s2, bpp);
      d1 += bpp;
    }
  }
  *this = dst;
}

void Image16::rotate90CCW() {
  Image16 dst(QSize(height(), width()), format());
  int bpp = bytesPerPixel();
  int X = dst.width();
  int Y = dst.height();
  int SL = bytesPerLine();
  int DL = dst.bytesPerLine();
  uchar const *s = bytes();
  uchar *d = dst.bytes();
  for (int y=0; y<Y; y++) {
    uchar const *s1 = s + bpp*(Y-1-y);
    uchar *d1 = d + DL*y;
    for (int x=0; x<X; x++) {
      uchar const *s2 = s1 + SL*x;
      memcpy(d1, s2, bpp);
      d1 += bpp;
    }
  }
  *this = dst;
}

void Image16::rotate180() {
  Image16 dst(size(), format());
  int X = width();
  int Y = height();
  int bpp = bytesPerPixel();
  int DL = bytesPerLine();
  uchar const *s = bytes();
  uchar *d = dst.bytes();
  for (int y=0; y<Y; y++) {
    uchar const *s1 = s + DL*(Y-1-y);
    uchar *d1 = d + DL*y;
    for (int x=0; x<X; x++) {
      uchar const *s2 = s1 + (X-1-x)*bpp;
      memcpy(d1, s2, bpp);
      d1 += bpp;
    }
  }
  *this = dst;
}

void Image16::crop(QRect r) {
  d->roibyteoffset += r.left()*bytesPerPixel() + r.top()*bytesPerLine();
  int roitop = d->roibyteoffset / bytesPerLine();
  int roileft = (d->roibyteoffset % bytesPerLine()) / bytesPerPixel();
  int imwidth = format()==Image16::Format::sRGB8 ? d->image.width()
    : (d->image.width()/3);
  int imheight = d->image.height();
  d->width = r.width();
  if (d->width + roileft > imwidth)
    d->width = imwidth - roileft;
  d->height = r.height();
  if (d->height + roitop > imheight)
    d->height = imheight - roitop;
  if (d->width<=0 || d->height<=0)
    *this = Image16();
}

Image16 Image16::cropped(QRect r) const {
  Image16 img = *this;
  img.crop(r);
  return img;
}

void Image16::applyROI() {
  int qwidth = (format()==Image16::Format::sRGB8) ? d->width : (d->width*3);
  int qheight = d->height;
  if (d->image.width()==qwidth && d->image.height()==qheight)
    return;

  int roitop = d->roibyteoffset / bytesPerLine();
  int roileft = (d->roibyteoffset % bytesPerLine()) / bytesPerPixel();
  d->image = d->image.copy(roileft, roitop, qwidth, qheight);
  d->roibyteoffset = 0;
  d->bytesperline = d->image.bytesPerLine();  
}
