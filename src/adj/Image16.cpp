// Image16.cpp

#include "Image16.h"
#include "PDebug.h"
#include "ColorSpaces.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <QFile>
#include "PPM16.h"

class Image16Foo {
public:
  Image16Foo() {
    qRegisterMetaType<Image16>("Image16");
  }
};

static Image16Foo foo;


Image16::Image16(): Image16(QImage()) {
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
  d(new Image16Data(width, height, format)) {
}

Image16::Image16(PSize size, Image16::Format format):
  Image16(size.width(), size.height(), format) {
}

Image16::Image16(uchar const *data, int width, int height,
                 Image16::Format format) {
  if (format==Format::sRGB8) 
    d = new Image16Data(QImage(data, width, height, QImage::Format_RGB32));
  else
    d = new Image16Data(QImage(data, width*3, height, QImage::Format_RGB16));
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
  d->width = width;
  d->format = format;
}

Image16 &Image16::operator=(Image16 const &image) {
  d = image.d;
  return *this;
}

QImage Image16::toQImage() const {
  if (format()==Format::sRGB8) {
    if (d->width==d->image.width() && d->height==d->image.height())
      return d->image;
    Image16 a = *this;
    a.applyROI();
    return a.toQImage();
  } else {
    return convertedTo(Format::sRGB8).toQImage();
  }
}

Image16::Image16(QImage const &image): d(new Image16Data(image)) {
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
      qDebug() << "Converting from IPT16";
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
    if (f==Format::Lab16 || f==Format::IPT16) {
      return scaleSigned(s, i);
    }     
  }

  // So now we _know_ that we have unsigned data, or that it doesn't matter
  int maxdecimation
    = i==Interpolation::Linear ? 2
    : i==Interpolation::Area ? 2
    : i==Interpolation::Cubic ? 3
    : i==Interpolation::Lanczos ? 6
    : 1000000;

  if (width()>maxdecimation*s.width() && height()>maxdecimation*s.height()) {
    Image16 img = scaled(PSize((width()+1)/2, (height()+1)/2),
                         Interpolation::Linear);
    return img.scaled(s, i);
  }

  int cvfmt = cvFormat(format());
  cv::Mat const in(height(), width(), cvfmt, (void*)bytes(), bytesPerLine());
  Image16 res(s, format());
  cv::Mat out(s.height(), s.width(), cvfmt, res.bytes(), res.bytesPerLine());
  cv::resize(in, out, out.size(), 0, 0, cvInterpolation(i));
  return res;
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

void Image16::rotate90CCW() {
  Image16 dst(PSize(height(), width()), format());
  cv::Mat const in(height(), width(), cvFormat(format()),
                   (void*)bytes(), bytesPerLine());
  cv::Mat out(dst.height(), dst.width(), cvFormat(format()),
              (void*)dst.bytes(), dst.bytesPerLine());
  cv::transpose(in, out);
  cv::flip(out, out, 1);
  *this = dst;
}

void Image16::rotate90CW() {
  Image16 dst(PSize(height(), width()), format());
  cv::Mat const in(height(), width(), cvFormat(format()),
                   (void*)bytes(), bytesPerLine());
  cv::Mat out(dst.height(), dst.width(), cvFormat(format()),
              (void*)dst.bytes(), dst.bytesPerLine());
  cv::transpose(in, out);
  cv::flip(out, out, 0);
  *this = dst;
}

void Image16::rotate180() {
  cv::Mat in(height(), width(), cvFormat(format()),
             (void*)bytes(), bytesPerLine());
  cv::flip(in, in, -1);
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

Image16 Image16::scaleSigned(PSize s, Image16::Interpolation i) const {
  /* Certain of our formats have signed data in the second and third channels.
     That doesn't work when scaling. So we convert to unsigned by shifting
     nominal zero to 0x8000.
   */
  Image16 img = *this;
  img.flipSignedness();
  img.d->format = Format::XYZp16; // just pretending
  img = img.scaled(s, i);
  img.flipSignedness();
  img.d->format = format();
  return img;
}

Image16 Image16::rotateSigned(double angle, Image16::CropMode c,
                              Image16::Interpolation i) const {
  Image16 img = *this;
  img.flipSignedness();
  img.d->format = Format::XYZp16; // just pretending
  img = img.rotated(angle, c, i);
  img.flipSignedness();
  img.d->format = format();
  return img;
}

Image16 Image16::perspectiveSigned(QPolygonF corners, Image16::CropMode c,
                                   Image16::Interpolation i) const {
  Image16 img = *this;
  img.flipSignedness();
  img.d->format = Format::XYZp16; // just pretending
  img = img.perspectived(corners, c, i);
  img.flipSignedness();
  img.d->format = format();
  return img;
}

void Image16::flipSignedness() {
  quint16 *ptr = words();
  int W = width();
  int H = height();
  int DL = wordsPerLine() - 3*W;
  for (int y=0; y<H; y++) {
    for (int x=0; x<W; x++) {
      ptr++;
      *ptr++ ^= 32768;
      *ptr++ ^= 32768;
    }
    ptr += DL;
  }
}

int Image16::cvFormat(Image16::Format f) {
  return f==Format::sRGB8 ? CV_8UC4 : CV_16UC3;
}

int Image16::cvInterpolation(Image16::Interpolation i) {
  return i==Interpolation::NearestNeighbor ? cv::INTER_NEAREST
    : i==Interpolation::Linear ? cv::INTER_LINEAR
    : i==Interpolation::Area ? cv::INTER_AREA
    : i==Interpolation::Cubic ? cv::INTER_CUBIC
    : i==Interpolation::Lanczos ? cv::INTER_LANCZOS4
    : cv::INTER_LINEAR; // default
}

Image16 Image16::rotated(double angle, Image16::CropMode c,
                         Image16::Interpolation i) const {
  if (isNull() || angle==0)
    return *this;

  if (i!=Interpolation::NearestNeighbor) {
    Format f = format();
    if (f==Format::Lab16 || f==Format::IPT16)
      return rotateSigned(angle, c, i);
  }

  // So now we _know_ that we have unsigned data, or that it doesn't matter
  int cvfmt = cvFormat(format());
  cv::Mat const in(height(), width(), cvfmt, (void*)bytes(), bytesPerLine());

  // For now, we pretend that crop mode is Same...
  cv::Mat rot = cv::getRotationMatrix2D(cv::Point2f(width()/2.0, height()/2.0),
                                        -180*angle/M_PI, 1);
  Image16 res(size(), format());
  cv::Mat out(height(), width(), cvfmt, res.bytes(), res.bytesPerLine());
  cv::warpAffine(in, out, rot, out.size(),
                 cvInterpolation(i) | cv::WARP_INVERSE_MAP,
                 cv::BORDER_CONSTANT, cv::Scalar());
  // I should specify the value of that scalar.
  return res;
}

Image16 Image16::perspectived(QPolygonF poly, Image16::CropMode,
                              Image16::Interpolation i) const {
  if (poly.size()!=4) {
    qDebug() << "Image16::perspectived: need 4-gon";
    return Image16();
  }

  if (i!=Interpolation::NearestNeighbor) {
    if (i!=Interpolation::Linear)
      qDebug() << "Note: Image16::perspectived only supports "
        "up to linear interpolation";
    i = Interpolation::Linear;
  }
  
  int cvfmt = cvFormat(format());
  cv::Mat const in(height(), width(), cvfmt, (void*)bytes(), bytesPerLine());

  cv::Point2f src[4];
  cv::Point2f dst[4];
  for (int k=0; k<4; k++)
    dst[k] = cv::Point2f(poly[k].x(), poly[k].y());
  src[0] = cv::Point2f(0, 0);
  src[1] = cv::Point2f(width(), 0);
  src[2] = cv::Point2f(0, height());
  src[3] = cv::Point2f(width(), height());
 
  cv::Mat per = cv::getPerspectiveTransform(src, dst);
  Image16 res(size(), format());
  cv::Mat out(height(), width(), cvfmt, res.bytes(), res.bytesPerLine());
  cv::warpPerspective(in, out, per, out.size(),
                      cvInterpolation(i) | cv::WARP_INVERSE_MAP,
                      cv::BORDER_CONSTANT, cv::Scalar());
  // I should specify the value of that scalar.
  return res;
}

Image16 Image16::loadFromFile(QString const &fn) {
  QFile f(fn);
  if (!f.open(QFile::ReadOnly))
    return Image16();
  QByteArray ar = f.readAll();
  return loadFromMemory(ar);
}

Image16 Image16::loadFromMemory(QByteArray const &ar) {
  PPM16 ppm(ar);
  if (!ppm.ok())
    return Image16(QImage::fromData(ar));
  qDebug() << "loadfrommemory";

  Image16 res;
  res.d = new Image16Data(ppm.data(), Format::XYZ16);
  return res;
}
