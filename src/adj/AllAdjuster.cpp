// AllAdjuster.cpp

#include "AllAdjuster.h"
#include "PDebug.h"

AllAdjuster::AllAdjuster(QObject *parent): Adjuster(parent) {
  validInputUntil = -1;
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
  validInputUntil = -1;
  lastrq = AllAdjustments();
}

bool AllAdjuster::isEmpty() const {
  return Adjuster::isEmpty();
  // should I actually look at other layers?
}

void AllAdjuster::setOriginal(Image16 const &image) {
  pDebug() << "AllAdjuster::setOriginal" << image.size();
  Adjuster::setOriginal(image);
  validInputUntil = 0;
}

void AllAdjuster::setReduced(Image16 const &image, PSize originalSize) {
  pDebug() << "AllAdjuster::setReduced" << image.size() << originalSize;
  Adjuster::setReduced(image, originalSize);
  validInputUntil = 0;
}

Image16 AllAdjuster::retrieveFull(AllAdjustments const &settings) {
  pDebug() << "AllAdjuster::retrieveFull - UNTESTED";
  if (validInputUntil<0)
    return Image16(); // no image set at all
  int N = settings.layerCount();
  ensureAdjusters(N);
  int n = 0;
  /* We need to find the greatest n such that settings are unchanged for
     all k<n */
  if (validInputUntil>0 && settings.baseAdjustments()==lastrq.baseAdjustments()) {
    n = 1;
    while (n<validInputUntil
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
    validInputUntil = n;
  }
  pDebug() << "AA::retrieveFull returning layered img sized " << img.size();
  return img;
}

Image16 AllAdjuster::retrieveReduced(AllAdjustments const &settings,
				     PSize maxSize) {
  pDebug() << "AllAdjuster::retrieveReduced" << maxSize;
  pDebug() <<  "  settings" << settings;
  if (validInputUntil<0)
    return Image16(); // no image set at all
  int N = settings.layerCount();
  ensureAdjusters(N);
  // See notes p. 71 dd 7/28
  int F = 1;
  /* To find Q: Of course I could look at maxAvailableSize for layer
     n, but that is not quite the relevant thing, because
     maxAvailableSize for a previous layer might be too restrictive. I
     think the logic should be that if mAS_n is insufficient for the
     request _and_ mAS_n is less than mAS_(n-1), then, we need to
     re-supply the input to n.  Specifically, if mAS_1 is insufficient
     and less than mAS_0, we need to reprovide input to layer 1. 
  */
  QMap<int, PSize> mAS; // max retrievable size for layer n (n=0..N, inclusive)
  mAS[0] = layerAdjuster(0)->maxAvailableSize(settings.baseAdjustments());
  for (int n=1; n<=N; n++)
    mAS[n] = layerAdjuster(n)->inputSize();

  auto noSizeLimit = [=](int f) {
    ASSERT(f>=1 && f<=N);
    return mAS[f].contains(maxSize) || mAS[f]==mAS[f-1];
  };
  auto noSettingsChange = [=](int f) {
    if (f==0)
      return settings.baseAdjustments()==lastrq.baseAdjustments();
    ASSERT(f>=1 && f<=N);
    return settings.layerAdjustments(f)==lastrq.layerAdjustments(f);
  };
  auto noMaskChange = [=](int f) {
    if (f==0)
      return true;
    ASSERT(f>=1 && f<=N);
    return settings.layer(f)==lastrq.layer(f);
  };
  
  while (F < N+1
	 && F <= validInputUntil
	 && noSizeLimit(F)
	 && noMaskChange(F-1)
	 && noSettingsChange(F-1))
    ++F;

  auto getImageAt = [=](int f) {
    Adjuster *adj = f==0 ? this : layerAdjuster(f);
    return adj->retrieveReduced(f==0
				? settings.baseAdjustments()
				: settings.layerAdjustments(f),
				maxSize);
  };

  auto applyMask = [=](Image16 const &img_top, Image16 const &img_below,
		       Layer const &) {
    //    QImage msk ....
    pDebug() << "AllAdjuster applyMask NYI";
    return img_top;
  };
  
  Image16 imgF1, imgF2;
  while (true) {
    if (F-2>=0 && imgF2.isNull())
      imgF2 = getImageAt(F-2);
    imgF1 = getImageAt(F-1);
    Image16 img = F==1 ? imgF1 : applyMask(imgF1, imgF2, settings.layer(F-1));
    if (F>N) {
      pDebug() << "AA::retrieveReduced returning layered img sized "
	       << img.size();
      return img;
    } else {
      layerAdjuster(F)->setOriginal(img);
      validInputUntil = F;
      imgF2 = imgF1;
      ++F;
    }
  }
  return Image16(); // not executed
}

Image16 AllAdjuster::retrieveROI(AllAdjustments const &settings, QRect roi) {
  pDebug() << "AA::retrieveROI - INADEQUATE IMPLEMENTATION";
  return Adjuster::retrieveROI(settings.baseAdjustments(), roi);
  // Grossly inadequate, of course
}
  
Image16 AllAdjuster::retrieveReducedROI(AllAdjustments const &settings,
					QRect roi, PSize maxSize) {
  pDebug() << "AA::retrieveReducedROI - INADEQUATE IMPLEMENTATION";
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

