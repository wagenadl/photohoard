// AllAdjuster.cpp

#include "AllAdjuster.h"
#include "PDebug.h"

AllAdjuster::AllAdjuster(QObject *parent): Adjuster(parent) {
  validUntil = -1;
}

AllAdjuster::~AllAdjuster() {
  for (Adjuster *a: layerAdjusters_)
    delete a;
}

Adjuster const *AllAdjuster::layerAdjuster(int n) const {
  if (n==0)
    return this;
  int k = n - 1;
  ASSERT(k>=0 && k<layerAdjusters_.size());
  return layerAdjusters_[k];
}

Adjuster *AllAdjuster::layerAdjuster(int n) {
  if (n==0)
    return this;
  int k = n - 1;
  ASSERT(k>=0 && k<layerAdjusters_.size());
  return layerAdjusters_[k];
}

void AllAdjuster::setMaxThreads(int m) {
  Adjuster::setMaxThreads(m);
  for (Adjuster *a: layerAdjusters_)
    a->setMaxThreads(m);
}

void AllAdjuster::clear() {
  Adjuster::clear();
  for (Adjuster *a: layerAdjusters_)
    a->clear();
  validUntil = -1;
  lastrq = AllAdjustments();
}

bool AllAdjuster::isEmpty() const {
  return Adjuster::isEmpty();
  // should I actually look at other layers?
}

void AllAdjuster::setOriginal(Image16 const &image) {
  pDebug() << "AllAdjuster::setOriginal" << image.size();
  Adjuster::setOriginal(image);
  validUntil = 0;
}

void AllAdjuster::setReduced(Image16 const &image, PSize originalSize) {
  pDebug() << "AllAdjuster::setReduced" << image.size() << originalSize;
  Adjuster::setReduced(image, originalSize);
  validUntil = 0;
}

Image16 AllAdjuster::retrieveFull(AllAdjustments const &settings) {
  pDebug() << "AllAdjuster::retrieveFull";
  if (validUntil<0)
    return Image16(); // no image set at all
  int N = settings.layerCount();
  ensureAdjusters(N);
  int n = 0;
  /* We need to find the greatest n such that settings are unchanged for
     all k<n */
  if (validUntil>0 && settings.baseAdjustments()==lastrq.baseAdjustments()) {
    n = 1;
    while (n<validUntil
	   && settings.layerAdjustments(n)==lastrq.layerAdjustments(n))
      ++n;
  }
  // n is the first layer where anything has changed.
  // We know that layer n has valid input.
  Image16 img;
  while (n<=N) {
    if (n==0) 
      img = Adjuster::retrieveFull(settings.baseAdjustments());
    else
      img = layerAdjuster(n)->retrieveFull(settings.layerAdjustments(n));
    n++;
    if (n<=N)
      layerAdjuster(n)->setOriginal(img);
    validUntil = n;
  }
  pDebug() << "AA::retrieveFull returning layered img sized " << img.size();
  return img;
}

Image16 AllAdjuster::retrieveReduced(AllAdjustments const &settings,
				     PSize maxSize) {
  pDebug() << "AllAdjuster::retrieveReduced" << maxSize;
  pDebug() <<  "  settings" << settings;
  if (validUntil<0)
    return Image16(); // no image set at all
  int N = settings.layerCount();
  ensureAdjusters(N);
  int n = 0;
  /* We need to find the greatest n such that (1) settings are unchanged for
     all k<n and (2) the input image for layer n is large enough.
     (1) is just as for retrieveFull. (2) is tripping me up. Of course I
     could look at maxAvailableSize for layer n, but that is not quite the
     relevant thing, because maxAvailableSize for a previous layer might
     be too restrictive. I think the logic should be that if mAS_n is
     insufficient for the request _and_ mAS_n is less than mAS_(n-1), then,
     we need to re-supply the input to n.
     Specifically, if mAS_1 is insufficient and less than mAS_0, we need
     to reprovide input to layer 1. In that case, final n should be 0.
  */
  QMap<int, PSize> mAS;
  mAS[0] = layerAdjuster(0)->maxAvailableSize(settings.baseAdjustments());
  for (int n=1; n<=settings.layerCount(); n++)
    mAS[n] = layerAdjuster(n)->maxAvailableSize(settings.layerAdjustments(n));
  if (validUntil>0 && settings.baseAdjustments()==lastrq.baseAdjustments()
      && (mAS[1].contains(maxSize) || mAS[1]==mAS[0])) {
    n = 1;
    while (n<validUntil
	   && settings.layerAdjustments(n)==lastrq.layerAdjustments(n)
	   && (mAS[n+1].contains(maxSize) || mAS[n+1]==mAS[n]))
      ++n;
  }
  // n is the first layer where anything has changed.
  // We know that layer n has valid input.
  // Or, n is the first layer that needs to be recomputed because its
  // previous output was not large enough
  Image16 img;
  while (n<=N) {
    if (n==0) 
      img = Adjuster::retrieveReduced(settings.baseAdjustments(), maxSize);
    else
      img = layerAdjuster(n)->retrieveReduced(settings.layerAdjustments(n),
					      maxSize);
    n++;
    if (n<=N)
      layerAdjuster(n)->setOriginal(img);
    validUntil = n;
  }
  pDebug() << "AA::retrieveReduced returning layered img sized " << img.size();
  return img;
}

Image16 AllAdjuster::retrieveROI(AllAdjustments const &settings, QRect roi) {
  pDebug() << "AA::retrieveROI";
  return Adjuster::retrieveROI(settings.baseAdjustments(), roi);
  // Grossly inadequate, of course
}
  
Image16 AllAdjuster::retrieveReducedROI(AllAdjustments const &settings,
					QRect roi, PSize maxSize) {
  pDebug() << "AA::retrieveReducedROI";
  return Adjuster::retrieveReducedROI(settings.baseAdjustments(), roi, maxSize);
  // Grossly inadequate, of course
}

void AllAdjuster::enableCaching(bool ec) {
  Adjuster::enableCaching(ec);
  for (Adjuster *a: layerAdjusters_)
    a->enableCaching(ec);
}

void AllAdjuster::disableCaching() {
  enableCaching(false);
}

void AllAdjuster::preserveOriginal(bool po) {
  Adjuster::preserveOriginal(po);
  for (Adjuster *a: layerAdjusters_)
    a->preserveOriginal(po);
}

void AllAdjuster::cancel() {
  Adjuster::cancel();
  for (Adjuster *a: layerAdjusters_)
    a->cancel();
}

void AllAdjuster::ensureAdjusters(int nLayers) {
  while (layerAdjusters_.size() < nLayers) {
    pDebug() << "AllAdjuster: creating new Adjuster";
    Adjuster *adj = new Adjuster(this);
    pDebug() << "AA: done" << adj << this << adj->parent();
    adj->enableCaching(isCaching());
    adj->preserveOriginal(preservesOriginal());
    adj->setMaxThreads(maxThreads());
    layerAdjusters_ << adj;
  }
}

