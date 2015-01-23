// CMSTransform.h

#ifndef CMSTRANSFORM_H

#define CMSTRANSFORM_H

#include <lcms2.h>
#include <QString>
#include "CMSProfile.h"
#include <QImage>

class CMSTransform {
public:
  enum class ImageFormat {
    uInt8_4, // four bytes, backwards, i.e., QImage::Format_ARGB
    uInt8, // three bytes
    uInt16, // three shorts
    Float // three floats
      // Caution: Float makes for very slow transforms: no LUT is made
    };
  enum class RenderingIntent {
    Perceptual = INTENT_PERCEPTUAL,
    RelativeColorimetric = INTENT_RELATIVE_COLORIMETRIC,
    Saturation = INTENT_SATURATION,
    AbsoluteColorimetric = INTENT_ABSOLUTE_COLORIMETRIC,
    };
public:
  CMSTransform();
  CMSTransform(CMSProfile const &input, 
               CMSProfile const &output,
               ImageFormat inputfmt=ImageFormat::uInt8_4,
               ImageFormat outputfmt=ImageFormat::uInt8_4,
               RenderingIntent intent=RenderingIntent::Perceptual);
  // I probably want one also for QImage::Format values as image format?
  CMSTransform(CMSTransform const &);
  CMSTransform &operator=(CMSTransform const &);
  virtual ~CMSTransform();
  QImage apply(QImage const &) const; // null if format mismatch
  void apply(void *dest, void const *source, int npixels) const;
  bool isValid() const;
private:
  void initref();
  void ref();
  void deref();
  static int format(CMSProfile const &profile, ImageFormat fmt);
private:
  class QAtomicInt *refctr;
  cmsHTRANSFORM xform;
private:
  CMSProfile input, output;
  ImageFormat inputfmt, outputfmt;
  RenderingIntent intent;
};

#endif
