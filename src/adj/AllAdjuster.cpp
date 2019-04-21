// AllAdjuster.cpp

#include "AllAdjuster.h"

AllAdjuster::AllAdjuster(QObject *parent): Adjuster(parent) {
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
  for (bool &b: validInput)
    b = false;
  lastrq = AllAdjustments();
}

bool AllAdjuster::isEmpty() const {
  return Adjuster::isEmpty();
  // should I actually look at other layers?
}

void AllAdjuster::setOriginal(Image16 const &image) {
  Adjuster::setOriginal(image);
  for (bool &b: validInput)
    b = false;
}

void AllAdjuster::setReduced(Image16 const &image, PSize originalSize) {
  Adjuster::setReduced(image, originalSize);
  for (bool &b: validInput)
    b = false;
}

Image16 AllAdjuster::retrieveFull(AllAdjustments const &settings) {
  return Adjuster::retrieveFull(settings.baseAdjustments());
  // Grossly inadequate, of course
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
    validInput << false;
  }
}

