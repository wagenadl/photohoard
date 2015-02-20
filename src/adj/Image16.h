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
  Image16(uint width, uint height, Format format=Format::sRGB8);
  Image16(QSize size, Format format=Format::sRGB8);
  Image16(uchar const *data, uint width, uint height,
          Format format=Format::sRGB8);
  Image16(uchar const *data, uint width, uint height, uint bytesPerLine,
          Format format=Format::sRGB8);
  Image16 &operator=(Image16 const &image);
  QImage toQImage() const;
  static Image16 fromQImage(QImage const &image);
public:
  Image16 convertedTo(Format format) const;
  void convertTo(Format format);
  void convertFrom(Image16 const &other);
public:
  inline uint width() const { return d->width; }
  inline uint height() const { return d->height; }
  inline uint bytesPerLine() const { return d->bytesperline; }
  inline uint wordsPerLine() const { return d->bytesperline/2; }
  inline uint byteCount() const { return bytesPerLine()*height(); }
  inline QSize size() const { return QSize(width(), height()); }
  inline Format format() const { return d->format; }
  inline bool is8Bits() const { return format() == Format::sRGB8; }
  inline bool isNull() const { return height()==0; }
  inline uchar const *bytes() const { return d->image.bits(); }
  inline uchar *bytes() { return d->image.bits(); }
  inline quint16 const *words() const { return (quint16 const *)bytes(); }
  inline quint16 *words() { return (quint16 *)bytes(); }
private:
  void convertFrom(Image16 const &other, Format otherformat);
private:
  class Data: public QSharedData {
  public:
    Data(uint w=0, uint h=0, Image16::Format f=Image16::Format::sRGB8):
      width(w), height(h), format(f),
      image(format==Image16::Format::sRGB8 ? w : 3*w, h,
            format==Image16::Format::sRGB8 ? QImage::Format_RGB32
            : QImage::Format_RGB16) {
      bytesperline = image.bytesPerLine();
    }
    Data(QImage const &img):
      width(img.width()), height(img.height()),
      bytesperline(img.bytesPerLine()),
      format(Image16::Format::sRGB8), image(img) {
    }
  public:
    uint width;
    uint height;
    uint bytesperline;
    Format format;
    QImage image;
  };
  QSharedDataPointer<Data> d;
};

#endif
