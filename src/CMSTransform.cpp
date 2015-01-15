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
                           CMSTransform::RenderingIntent intent):
  input(input), output(output),
  inputfmt(inputfmt), outputfmt(outputfmt),
  intent(intent) {
  initref();
  xform = cmsCreateTransform(input.profile(), int(inputfmt),
                             output.profile(), int(outputfmt),
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
