// Image16.h

#ifndef IMAGE16_H

#define IMAGE16_H

#include <QImage>
#include <QMetaType>
#include <QSharedDataPointer>
#include "PSize.h"
#include "Image16Base.h"
#include "Image16Data.h"
#include <QPolygonF>

class Image16: public Image16Base {
public:
  Image16();
  Image16(QString const &fn, char const *format=0);
  Image16(char const *fn, char const *format=0);
  Image16(Image16 const &image);
  Image16(QImage const &image);
  Image16(int width, int height, Format format=Format::sRGB8);
  Image16(PSize size, Format format=Format::sRGB8);
  Image16(uchar const *data, int width, int height,
          Format format=Format::sRGB8);
  Image16(uchar const *data, int width, int height, int bytesPerLine,
          Format format=Format::sRGB8);
  Image16 &operator=(Image16 const &image);
  QImage toQImage() const;
  static Image16 fromQImage(QImage const &image);
  static Image16 loadFromFile(QString const &);
  static Image16 loadFromMemory(QByteArray const &);
public:
  Image16 convertedTo(Format format, int maxthreads=1) const;
  void convertTo(Format format, int maxthreads=1);
  void convertFrom(Image16 const &other, int maxthreads=1);
public:
  Image16 scaled(PSize s, Interpolation i=Interpolation::Linear) const;
  Image16 scaledToFitSnuglyIn(PSize s,
			      Interpolation i=Interpolation::Linear) const;
  Image16 scaledDownToFitIn(PSize s,
                            Interpolation i=Interpolation::Linear) const;
  Image16 scaledToWidth(int w, Interpolation i=Interpolation::Linear) const;
  Image16 scaledToHeight(int h, Interpolation i=Interpolation::Linear) const;
  Image16 cropped(QRect) const;
  Image16 rotated(double angle, 
                  Interpolation i=Interpolation::Linear) const;
  /* Rotates by the given angle, specified in counterclockwise radians. */
  Image16 perspectived(QPolygonF corners, 
                       Interpolation i=Interpolation::Linear) const;
  /* Produces a version where the polygon (which must have *precisely*
     four vertices) is transformed to a rectangle. */
  Image16 alphablend(Image16 ontop, QImage mask) const;
  /* The ontop image and the mask must have the same size as we do.
     mask must be grayscale8. mask=255 means full use of ontop,
     mask=0 means full use of our pixel. */
  void rotate90CW();
  void rotate90CCW();
  void rotate180();
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
  Image16 subImage(QRect);
  /* A sub-image is an Image16 that shares its memory with its source, without
     copy-on-write semantics. All operations performed on the subimage will
     directly affect the source image. The user is responsible for making
     sure that the source image remains in existence. The following methods
     must never be called on subimages or on parent images of subimages:
     rotate90(C)CW, applyROI, convertTo (with a different bit depth),
     operator=.
   */
public:
  inline int width() const { return d->width; }
  inline int height() const { return d->height; }
  inline int bytesPerLine() const { return d->bytesperline; }
  inline int wordsPerLine() const { return d->bytesperline/2; }
  inline int bytesPerPixel() const { return is8Bits() ? 4 : 6; }
  inline int byteCount() const { return bytesPerLine()*height(); }
  inline PSize size() const { return PSize(width(), height()); }
  inline Format format() const { return d->format; }
  inline bool is8Bits() const { return format() == Format::sRGB8; }
  inline bool isNull() const { return height()==0; }
  inline uchar const *bytes() const { return d->image.bits()+d->roibyteoffset; }
  inline uchar *bytes() { return d->image.bits()+d->roibyteoffset; }
  inline quint16 const *words() const { return (quint16 const *)bytes(); }
  inline quint16 *words() { return (quint16 *)bytes(); }
private:
  void convertFrom(Image16 const &other, Format otherformat, int maxthreads=1);
  void setROI(QRect);
  Image16 scaleSigned(PSize s, Interpolation i) const;
  Image16 rotateSigned(double angle,
                       Interpolation i=Interpolation::Linear) const;
  /* Rotates by the given angle, specified in counterclockwise radians. */
  Image16 perspectiveSigned(QPolygonF corners, 
                            Interpolation i=Interpolation::Linear) const;
  void flipSignedness();
  Image16(Image16 *src, QRect sub);
public:
  static int cvFormat(Format f);
  static int cvInterpolation(Interpolation i);
private:
  QSharedDataPointer<Image16Data> d;
};

#endif
