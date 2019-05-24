// AllAdjuster.cpp

#include "AllAdjuster.h"

AllAdjuster::AllAdjuster(QObject *parent): Adjuster(parent) {
  validUntil = -1;
}

AllAdjuster::~AllAdjuster() {
  for (Adjuster *a: layerAdjusters)
    delete a;
}

void AllAdjuster::setMaxThreads(int m) {
  Adjuster::setMaxThreads(m);
  for (Adjuster *a: layerAdjusters)
    a->setMaxThreads(m);
}

void AllAdjuster::clear() {
  Adjuster::clear();
  for (Adjuster *a: layerAdjusters)
    a->clear();
  validUntil = -1;
  lastrq = AllAdjustments();
}

bool AllAdjuster::isEmpty() const {
  return Adjuster::isEmpty();
  // should I actually look at other layers?
}

void AllAdjuster::setOriginal(Image16 const &image) {
  Adjuster::setOriginal(image);
  validUntil = 0;
}

void AllAdjuster::setReduced(Image16 const &image, PSize originalSize) {
  Adjuster::setReduced(image, originalSize);
  validUntil = 0;
}

Image16 AllAdjuster::retrieveFull(AllAdjustments const &settings) {
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
      img = layerAdjusters[n-1]->retrieveFull(settings.baseAdjustments());
    n++;
    if (n<=N)
      layerAdjusters[n-1]->setOriginal(img);
    validUntil = n;
  }
  return img;
}

Image16 AllAdjuster::retrieveReduced(AllAdjustments const &settings,
				     PSize maxSize) {
  return Adjuster::retrieveReduced(settings.baseAdjustments(), maxSize);
  // Grossly inadequate, of course
}

Image16 AllAdjuster::retrieveROI(AllAdjustments const &settings, QRect roi) {
  return Adjuster::retrieveROI(settings.baseAdjustments(), roi);
  // Grossly inadequate, of course
}
  
Image16 AllAdjuster::retrieveReducedROI(AllAdjustments const &settings,
					QRect roi, PSize maxSize) {
  return Adjuster::retrieveReducedROI(settings.baseAdjustments(), roi, maxSize);
  // Grossly inadequate, of course
}

void AllAdjuster::enableCaching(bool ec) {
  Adjuster::enableCaching(ec);
  for (Adjuster *a: layerAdjusters)
    a->enableCaching(ec);
}

void AllAdjuster::disableCaching() {
  enableCaching(false);
}

void AllAdjuster::preserveOriginal(bool po) {
  Adjuster::preserveOriginal(po);
  for (Adjuster *a: layerAdjusters)
    a->preserveOriginal(po);
}

void AllAdjuster::cancel() {
  Adjuster::cancel();
  for (Adjuster *a: layerAdjusters)
    a->cancel();
}

void AllAdjuster::ensureAdjusters(int nLayers) {
  while (layerAdjusters.size() < nLayers) {
    Adjuster *adj = new Adjuster(this);
    adj->enableCaching(isCaching());
    adj->preserveOriginal(preservesOriginal());
    adj->setMaxThreads(maxThreads());
    layerAdjusters << adj;
  }
}

