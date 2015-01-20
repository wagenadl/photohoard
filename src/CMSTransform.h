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
    ARGB = TYPE_BGRA_8, // this is QImage::Format_ARGB32 or _RGB32: 0xaarrggbb.
    // etc.
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
               ImageFormat inputfmt=ImageFormat::ARGB,
               ImageFormat outputfmt=ImageFormat::ARGB,
               RenderingIntent intent=RenderingIntent::Perceptual);
  // I probably want one also for QImage::Format values as image format?
  CMSTransform(CMSTransform const &);
  CMSTransform &operator=(CMSTransform const &);
  virtual ~CMSTransform();
  QImage apply(QImage const &) const; // null if format mismatch
  bool isValid() const;
private:
  void initref();
  void ref();
  void deref();
private:
  class QAtomicInt *refctr;
  cmsHTRANSFORM xform;
private:
  CMSProfile input, output;
  ImageFormat inputfmt, outputfmt;
  RenderingIntent intent;
};

#endif
