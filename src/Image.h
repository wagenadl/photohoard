// Image.h

#ifndef IMAGE_H

#define IMAGE_H

class Image {
public:
  enum class Format {
    Gray8,
    Color8, // that's QImage::Format_RGB32
    Gray16,
    Color16,
  };
  enum class Space {
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
  Image(Image const &image);
  Image(uint width, uint height,
        Format format=Format::Color8, Space space=Space::sRGB);
  Image(uchar const *data, uint width, uint height,
        Format format=Format::Color8, Space space=Space::sRGB);        
  Image(uchar const *data, uint width, uint height, uint bytesPerLine,
        Format format=Format::Color8, Space space=Space::sRGB);
  Image &operator=(Image const &image);
  QImage toImage() const;
  static Image fromImage(QImage const &image);
  Image convertedToFormat(Format format) const;
  Image convertedToSpace(Space space) const;
  Image convertedTo(Format format, Space space) const;
  bool inPlaceConvertToSpace(Space space);
  operator QVariant() const;
  // Comparision
  bool operator==(Image const &) const;
  bool operator!=(Image const &) const;
  // Basic information
  uint width() const;
  uint height() const;
  QSize size() const;
  uint bytesPerLine() const;
  uint byteCount() const;
  Format format() const;
  Space space() const;
  bool isGrayscale() const;
  bool is8Bits() const;
  bool isNull() const;
  // Access functions
  uchar const *constBits() const;
  uchar const *bits() const;
  uchar *bits();
  uchar const *constScanLine(uint y) const;
  uchar const *scanLine(uint y) const;
  uchar *scanLine(uint y);
  uchar const *constPixel(uint x, uint y) const;
  uchar const *pixel(uint x, uint y) const;
  uchar *pixel(uint x, uint y);
  uchar const *constPixel(QPoint const &xy) const;
  uchar const *pixel(QPoint const &xy) const;
  uchar *pixel(QPoint const &xy);
  bool valid(uint x, uint y) const;
  bool valid(QPoint const &xy) const;
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
  QImage d;
  Format f;
  Space s;
};

QDataStream &operator<<(QDataStream &stream, Image const &image);
QDataStream &operator>>(QDataStream &stream, Image &image);

Q_DECLARE_METATYPE(Image);

#endif
