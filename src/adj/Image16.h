// Image16.h

#ifndef IMAGE16_H

#define IMAGE16_H

#include <QImage>
#include <QMetaType>
#include <QSharedDataPointer>
#include <QDebug>

class Image16 {
public:
  enum class Format {
    sRGB8,
    XYZ16,
    XYZp16,
    Lab16,
    LMS16,
    IPT16,
  };
public:
  Image16();
  Image16(QString const &fn, char const *format=0);
  Image16(char const *fn, char const *format=0);
  Image16(Image16 const &image);
  Image16(QImage const &image);
  Image16(int width, int height, Format format=Format::sRGB8);
  Image16(QSize size, Format format=Format::sRGB8);
  Image16(uchar const *data, int width, int height,
          Format format=Format::sRGB8);
  Image16(uchar const *data, int width, int height, int bytesPerLine,
          Format format=Format::sRGB8);
  Image16 &operator=(Image16 const &image);
  QImage toQImage() const;
  static Image16 fromQImage(QImage const &image);
public:
  Image16 convertedTo(Format format) const;
  void convertTo(Format format);
  void convertFrom(Image16 const &other);
public:
  Image16 scaled(QSize s, Qt::AspectRatioMode arm=Qt::IgnoreAspectRatio) const;
  Image16 scaledToWidth(int w,
                        Qt::TransformationMode tm=Qt::FastTransformation) const;
  Image16 scaledToHeight(int h,
                        Qt::TransformationMode tm=Qt::FastTransformation) const;
  void rotate90CW();
  void rotate90CCW();
  void rotate180();
  Image16 cropped(QRect) const;
  void crop(QRect);
  void applyROI();
  /* Cropping doesn't change the underlying QImage. Rather, it sets up an ROI.
     The main reason to "apply" that ROI and get a truly cropped image is to
     release the memory held in the unused areas.
     It is worth noting that convertTo() automatically applies the ROI
     if the bit depth changes.
     Conversely, convertedTo() does not apply the ROI if the target format
     equals the source format.
     applyROI() is trivially fast if there is no ROI.
   */
public:
  inline int width() const { return d->width; }
  inline int height() const { return d->height; }
  inline int bytesPerLine() const { return d->bytesperline; }
  inline int wordsPerLine() const { return d->bytesperline/2; }
  inline int bytesPerPixel() const { return is8Bits() ? 4 : 6; }
  inline int byteCount() const { return bytesPerLine()*height(); }
  inline QSize size() const { return QSize(width(), height()); }
  inline Format format() const { return d->format; }
  inline bool is8Bits() const { return format() == Format::sRGB8; }
  inline bool isNull() const { return !d || height()==0; }
  inline uchar const *bytes() const { return d->image.bits()+d->roibyteoffset; }
  inline uchar *bytes() { return d->image.bits()+d->roibyteoffset; }
  inline quint16 const *words() const { return (quint16 const *)bytes(); }
  inline quint16 *words() { return (quint16 *)bytes(); }
private:
  void convertFrom(Image16 const &other, Format otherformat);
  void setROI(QRect);
private:
  class Data: public QSharedData {
  public:
    Data(int w=0, int h=0, Image16::Format f=Image16::Format::sRGB8):
      width(w), height(h), format(f),
      image(format==Image16::Format::sRGB8 ? w : 3*w, h,
            format==Image16::Format::sRGB8 ? QImage::Format_RGB32
            : QImage::Format_RGB16), roibyteoffset(0) {
      bytesperline = image.bytesPerLine();
    }
    Data(QImage const &img):
      width(img.width()), height(img.height()),
      format(Image16::Format::sRGB8), image(img), roibyteoffset(0) {
      bytesperline = image.bytesPerLine();
    }
  public:
    int width;
    int height;
    int bytesperline;
    Format format;
    QImage image;
    int roibyteoffset;
  };
  QSharedDataPointer<Data> d;
};

#endif
