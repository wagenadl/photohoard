// Image.h

#ifndef IMAGE_H

#define IMAGE_H

#include <QImage>
#include <QMetaType>

class Image {
public:
  enum class Format {
    Color8, // that's QImage::Format_RGB32
    Color14,
  };
  enum class Space {
    // Color8 is always stored in sRGB
    sRGB,
    LinearRGB,
    XYZ,
    LabD50,
  };
  enum class TransformationMode {
    Fast=Qt::FastTransformation,
    NearestNeighbor=Qt::FastTransformation,
    Smooth=Qt::SmoothTransformation,
    Bilinear=Qt::SmoothTransformation,
    Bicubic,
    Lanczos,
  };
      
public:
  /* API is identical to QImage, to the extent possible. */
  // Constructors and format conversion
  Image();
  Image(QString const &fn, char const *format=0);
  Image(char const *fn, char const *format=0);
  Image(Image const &image);
  Image(uint width, uint height,
        Format format=Format::Color8, Space space=Space::sRGB);
  Image(QSize size,
        Format format=Format::Color8, Space space=Space::sRGB);
  Image(uchar const *data, uint width, uint height,
        Format format=Format::Color8, Space space=Space::sRGB);        
  Image(uchar const *data, uint width, uint height, uint bytesPerLine,
        Format format=Format::Color8, Space space=Space::sRGB);
  Image &operator=(Image const &image);
  QImage toQImage() const;
  static Image fromQImage(QImage const &image);
  Image convertedToFormat(Format format) const;
  Image convertedToSpace(Space space) const;
  Image convertedTo(Format format, Space space) const;
  void inPlaceConvertToSpace(Space space);
  operator QVariant() const;
  // Comparision
  bool operator==(Image const &) const;
  bool operator!=(Image const &) const;
  // Basic information
  uint width() const { return f==Format::Color14 ? d.width()/3 : d.width(); }
  uint height() const { return d.height(); }
  QSize size() const { return QSize(width(), height()); }
  uint bytesPerLine() const { return d.bytesPerLine(); }
  uint byteCount() const { return d.byteCount(); }
  Format format() const { return f; }
  Space space() const { return s; }
  bool is8Bits() const { return f==Format::Color8; }
  bool isNull() const { return d.isNull(); }
  // Access functions
  uchar const *constBits() const { return d.constBits(); }
  uchar const *bits() const { return d.bits(); }
  uchar *bits() { return d.bits(); }
  uchar const *constScanLine(uint y) const { return d.constScanLine(y); }
  uchar const *scanLine(uint y) const { return d.scanLine(y); }
  uchar *scanLine(uint y) { return d.scanLine(y); }
  bool valid(uint x, uint y) const { return x<width() && y<height(); }
  bool valid(QPoint const &xy) const { return valid(xy.x(), xy.y()); }
  // Loading and saving
  bool load(QString const &filename, char const *format=0);
  bool load(class QIODevice *device, char const *format=0);
  bool loadFromData(uchar const *data, int length, char const *format=0);
  bool loadFromData(QByteArray const &data, char const *format=0);
  static Image fromData(uchar const *data, int size, char const *format=0);
  static Image fromData(QByteArray const &data, char const *format=0);
  bool save(QString const &fileName,
	    char const *format=0, int quality=-1) const;
  bool save(QIODevice *device,
	    char const *format=0, int quality=-1 ) const;
  // In-place manipulation
  void invertPixels(QImage::InvertMode mode=QImage::InvertRgb);
  // Modified copies
  Image mirrored(bool horizontal=false, bool vertical=true) const;
  Image copy(QRect const &rectange=QRect()) const;
  Image copy(int x, int y, int width, int height) const;
  Image scaled(QSize const &size,
		   Qt::AspectRatioMode aspectRatioMode=Qt::IgnoreAspectRatio,
		   TransformationMode transformMode=TransformationMode::Fast)
    const;
  Image scaled(int width, int height,
		   Qt::AspectRatioMode aspectRatioMode=Qt::IgnoreAspectRatio,
		   TransformationMode transformMode=TransformationMode::Fast)
    const;
  Image scaledToHeight(int height,
		   TransformationMode transformMode=TransformationMode::Fast)
    const;
  Image scaledToWidth(int width,
		   TransformationMode transformMode=TransformationMode::Fast)
    const;
  Image transformed(QMatrix const &matrix,
		   TransformationMode transformMode=TransformationMode::Fast)
    const;
  Image transformed(QTransform const &matrix,
		   TransformationMode transformMode=TransformationMode::Fast)
    const;
  static QMatrix trueMatrix(QMatrix const &matrix, int width, int height);
  static QTransform trueMatrix(QTransform const &matrix, int width, int height);
private:
  void createGrayLut();
private:
  QImage d;
  Format f;
  Space s;
};

QDataStream &operator<<(QDataStream &stream, Image const &image);
QDataStream &operator>>(QDataStream &stream, Image &image);

Q_DECLARE_METATYPE(Image);

#endif
