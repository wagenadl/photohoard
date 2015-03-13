// Image16.cpp

#include "Image16.h"
#include <QDebug>
#include "ColorSpaces.h"
#include <opencv2/imgproc/imgproc.hpp>

class Image16Foo {
public:
  Image16Foo() {
    qRegisterMetaType<Image16>("Image16");
  }
};

static Image16Foo foo;


Image16::Image16(): Image16(QImage()) {
  //  qDebug() << this << d;
}

Image16::Image16(QString const &fn, char const *format):
  Image16(QImage(fn, format)) {
  //  qDebug() << this << d;
}

Image16::Image16(char const *fn, char const *format):
  Image16(QString(fn), format) {
  //  qDebug() << this << d;
}

Image16::Image16(Image16 const &image): d(image.d) {
  //  qDebug() << this << d;
}

Image16::Image16(int width, int height, Image16::Format format):
  d(new Image16Data(width, height, format)) {
  //  qDebug() << this << d;
}

Image16::Image16(PSize size, Image16::Format format):
  Image16(size.width(), size.height(), format) {
  //  qDebug() << this << d;
}

Image16::Image16(uchar const *data, int width, int height,
                 Image16::Format format) {
  if (format==Format::sRGB8) 
    d = new Image16Data(QImage(data, width, height, QImage::Format_RGB32));
  else
    d = new Image16Data(QImage(data, width*3, height, QImage::Format_RGB16));
  //  qDebug() << this << d;
  d->width = width;
  d->format = format;
}

Image16::Image16(uchar const *data, int width, int height, int bytesPerLine,
                 Image16::Format format) {
  if (format==Format::sRGB8) 
    d = new Image16Data(QImage(data, width, height, bytesPerLine,
                        QImage::Format_RGB32));
  else
    d = new Image16Data(QImage(data, width*3, height, bytesPerLine,
                        QImage::Format_RGB16));
  //  qDebug() << this << d;
  d->width = width;
  d->format = format;
}

Image16 &Image16::operator=(Image16 const &image) {
  d = image.d;
  //  qDebug() << this << d;
  return *this;
}

QImage Image16::toQImage() const {
  if (format()==Format::sRGB8)
    return d->image;
  else
    return convertedTo(Format::sRGB8).toQImage();
}

Image16::Image16(QImage const &image): d(new Image16Data(image)) {
  //  qDebug() << this << d;
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

Image16 Image16::scaled(PSize s, Image16::Interpolation i) const {
  if (isNull() || size()==s)
    return *this;
  else if (s.isEmpty())
    return Image16();

  if (i!=Interpolation::NearestNeighbor) {
    Format f = format();
    if (f==Format::Lab16 || f==Format::IPT16)
      return scaleSigned(s, i);
  }

  // So now we _know_ that we have unsigned data, or that it doesn't matter
  int cvfmt = format()==Format::sRGB8 ? CV_8UC4 : CV_16UC3;
  int maxdecimation
    = i==Interpolation::Linear ? 2
    : i==Interpolation::Area ? 2
    : i==Interpolation::Cubic ? 3
    : i==Interpolation::Lanczos ? 6
    : 1000000;

  if (width()>maxdecimation*s.width() && height()>maxdecimation*s.height()) 
    return scaled(PSize(width()/2, height()/2), Interpolation::Linear)
      .scaled(s, i);
  
  cv::Mat const in(height(), width(), cvfmt, (void*)bytes(), bytesPerLine());
  Image16 res(s, format());
  cv::Mat out(s.height(), s.width(), cvfmt, res.bytes(), res.bytesPerLine());
  int cvmeth
    = i==Interpolation::NearestNeighbor ? cv::INTER_NEAREST
    : i==Interpolation::Linear ? cv::INTER_LINEAR
    : i==Interpolation::Area ? cv::INTER_AREA
    : i==Interpolation::Cubic ? cv::INTER_CUBIC
    : i==Interpolation::Lanczos ? cv::INTER_LANCZOS4
    : cv::INTER_LINEAR; // default
  cv::resize(in, out, out.size(), 0, 0, cvmeth);
  return res;
}

Image16 Image16::scaleSigned(PSize s, Image16::Interpolation i) const {
  /* Certain of our formats have signed data in the second and third channels.
     That doesn't work when scaling. So we convert to unsigned by shifting
     nominal zero to 0x8000.
   */
  Format f0 = format();

  Image16 img = *this;

  quint16 *ptr = img.words();
  int W = img.width();
  int H = img.height();
  int DL = img.wordsPerLine() - 3*W;
  for (int y=0; y<H; y++) {
    for (int x=0; x<W; x++) {
      ptr++;
      *ptr++ ^= 32768;
      *ptr++ ^= 32768;
    }
    ptr += DL;
  }
  img.d->format = Format::XYZp16; // just pretending

  img = img.scaled(s, i);

  ptr = img.words();
  DL = img.wordsPerLine() - 3*W;
  for (int y=0; y<H; y++) {
    for (int x=0; x<W; x++) {
      ptr++;
      *ptr++ ^= 32768;
      *ptr++ ^= 32768;
    }
    ptr += DL;
  }
  img.d->format = f0;

  return img;
}
  
  

Image16 Image16::scaledToFitIn(PSize s, Image16::Interpolation i) const {
  return scaled(size().scaledToFitIn(s), i);
}

Image16 Image16::scaledDownToFitIn(PSize s, Image16::Interpolation i) const {
  return size().exceeds(s) ? scaledToFitIn(s, i) : *this;
}

Image16 Image16::scaledToWidth(int w, Image16::Interpolation i) const {
  return scaledToFitIn(PSize(w, 65536), i);
}

Image16 Image16::scaledToHeight(int h, Image16::Interpolation i) const {
  return scaledToFitIn(PSize(65536, h), i);
}

void Image16::rotate90CW() {
  Image16 dst(PSize(height(), width()), format());
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
  Image16 dst(PSize(height(), width()), format());
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
