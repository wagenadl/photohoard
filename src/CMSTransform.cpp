// CMSTransform.cpp

#include "CMSTransform.h"
#include <QAtomicInt>

void CMSTransform::initref() {
  refctr = new QAtomicInt;
  xform = NULL;
  ref();
}

void CMSTransform::deref() {
  if (!refctr->deref()) {
    // deleted last copy
    delete refctr;
    if (isValid())
      cmsDeleteTransform(xform);
  }
}

void CMSTransform::ref() {
  refctr->ref();
}

CMSTransform::CMSTransform() {
  initref();
}

CMSTransform::CMSTransform(CMSProfile const &input,
                           CMSProfile const &output,
                           CMSTransform::ImageFormat inputfmt,
                           CMSTransform::ImageFormat outputfmt,
                           CMSTransform::RenderingIntent intent,
			   bool adaptationState):
  input(input), output(output),
  inputfmt(inputfmt), outputfmt(outputfmt),
  intent(intent) {
  initref();
  cmsSetAdaptationState(adaptationState ? 1 : 0);
  xform = cmsCreateTransform(input.profile(), format(input, inputfmt),
                             output.profile(), format(output, outputfmt),
                             int(intent),
                             0);
}

CMSTransform::CMSTransform(CMSTransform const &o) {
  refctr = o.refctr;
  ref();
  xform = o.xform;
  
  input = o.input;
  output = o.output;
  inputfmt = o.inputfmt;
  outputfmt = o.outputfmt;
  intent = o.intent;
}

CMSTransform::~CMSTransform() {
  deref();
}

CMSTransform &CMSTransform::operator=(CMSTransform const &o) {
  if (o.refctr == refctr)
    return *this;

  deref();
  refctr = o.refctr;
  ref();
  xform = o.xform;
  
  input = o.input;
  output = o.output;
  inputfmt = o.inputfmt;
  outputfmt = o.outputfmt;
  intent = o.intent;

  return *this;
}

void CMSTransform::apply(void *dest, void const *source, int npixels) const {
  Q_ASSERT(xform);
  cmsDoTransform(xform, (uchar const *)source, (uchar *)dest, npixels);
}

QImage CMSTransform::apply(QImage const &src) const {
  if (!isValid())
    return QImage();
  switch (src.format()) {
  case QImage::Format_RGB32:
  case QImage::Format_ARGB32:
    break;
  default:
    return QImage();
  }
  QImage dst = src;
  uchar *d = dst.bits();
  uchar const *s = src.bits();
  int n = src.bytesPerLine() * src.height() / 4;
  cmsDoTransform(xform, s, d, n);
  return dst; 
}

bool CMSTransform::isValid() const {
  return xform!=NULL;
}

int CMSTransform::format(CMSProfile const &profile,
                         CMSTransform::ImageFormat fmt) {
  CMSProfile::ColorSpace space = profile.colorSpace();
  switch (fmt) {
  case ImageFormat::uInt8_4:
    switch (space) {
    case CMSProfile::ColorSpace::RGB: return TYPE_BGRA_8;
    default: return TYPE_BGRA_8; // oh well
    }
  case ImageFormat::uInt8:
    switch (space) {
    case CMSProfile::ColorSpace::RGB: return TYPE_RGB_8;
    case CMSProfile::ColorSpace::XYZ: return TYPE_RGB_8; // Oh well
    case CMSProfile::ColorSpace::Lab: return TYPE_Lab_8;
    default: return TYPE_RGB_8;
    }
  case ImageFormat::uInt16:
    switch (space) {
    case CMSProfile::ColorSpace::RGB: return TYPE_RGB_16;
    case CMSProfile::ColorSpace::XYZ: return TYPE_XYZ_16;
    case CMSProfile::ColorSpace::Lab: return TYPE_Lab_16;
    default: return TYPE_RGB_16;
    }
  case ImageFormat::Float:
    switch (space) {
    case CMSProfile::ColorSpace::RGB: return TYPE_RGB_FLT;
    case CMSProfile::ColorSpace::XYZ: return TYPE_XYZ_FLT;
    case CMSProfile::ColorSpace::Lab: return TYPE_Lab_FLT;
    default: return TYPE_RGB_FLT;
    }
  }
  return TYPE_RGB_8; // not executed
}

