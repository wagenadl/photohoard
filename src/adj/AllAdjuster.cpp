// AllAdjuster.cpp

#include "AllAdjuster.h"
#include "PDebug.h"
#include <cmath>
#include "PhotoOps.h"

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
  lastrq = settings;
  return img;
}

Image16 AllAdjuster::retrieveReduced(AllAdjustments const &settings,
				     PSize maxSize) {
  pDebug() << "AllAdjuster::retrieveReduced" << maxSize;
  pDebug() << "  Layercount" << settings.layerCount();
  pDebug() <<  "  settings" << settings;
  if (validInputUntil<0)
    return Image16(); // no image set at all
  int N = settings.layerCount();
  ensureAdjusters(N);
  // See notes p. 71 dd 7/28
  /* To find Q: Of course I could look at maxAvailableSize for layer
     n, but that is not quite the relevant thing, because
     maxAvailableSize for a previous layer might be too restrictive. I
     think the logic should be that if mAS_n is insufficient for the
     request _and_ mAS_n is less than mAS_(n-1), then, we need to
     re-supply the input to n.  Specifically, if mAS_1 is insufficient
     and less than mAS_0, we need to reprovide input to layer 1. 
  */

  auto prepMAS = [=]() {
    QList<PSize> mAS;    
    mAS << layerAdjuster(0)->maxAvailableSize(settings.baseAdjustments());
    for (int n=1; n<=N; n++)
      mAS << layerAdjuster(n)->inputSize();
    return mAS;
  };
    
  auto noSizeLimit = [=](int f) {
    static QList<PSize> mAS = prepMAS();
    // max retrievable size for layer n (n=0..N, inclusive)
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
  
  auto getImageAt = [=](int f) {
    Adjuster *adj = f==0 ? baseAdjuster() : layerAdjuster(f);
    return adj->retrieveReduced(f==0
				? settings.baseAdjustments()
				: settings.layerAdjustments(f),
				maxSize);
  };

  auto applyInpaint = [=](Image16 const &img_below, Layer const &layer) {
    PSize osize = originalSize();
    PSize sclcrpsize = img_below.size();
    QImage mask = layer.mask(osize, settings.baseAdjustments(), sclcrpsize);
    Image16 bot = img_below.convertedTo(Image16::Format::sRGB8); 
    double scl = layer.scale(osize, settings.baseAdjustments(), sclcrpsize);
    return PhotoOps::inpaint(bot, mask, 4.0*scl, 1); // radius?
  };

  auto applyMask = [=](Image16 const &img_top, Image16 const &img_below,
		       Layer const &layer) {
    PSize osize = originalSize();
    PSize sclcrpsize = img_top.size();
    if (img_below.size() != sclcrpsize) {
      if (!sclcrpsize.isEmpty()) 
        COMPLAIN("AllAdjuster applyMask: mismatching image sizes");
      return img_top;  
    }
    QImage mask = layer.mask(osize, settings.baseAdjustments(), sclcrpsize);
    Image16 top = img_top.convertedTo(Image16::Format::IPT16);
    Image16 bot = img_below.convertedTo(Image16::Format::IPT16);
    ASSERT(top.size()==bot.size());
    ASSERT(mask.size()==top.size());
    return bot.alphablend(top, mask);
  };

  int f = 1;
  while (f <= N
	 && f <= validInputUntil
	 && noSizeLimit(f)
	 && noMaskChange(f-1)
	 && noSettingsChange(f-1))
    f++;

  Image16 imgF1, imgF2;
  while (true) {
    if (f-2>=0 && imgF2.isNull())
      imgF2 = getImageAt(f-2);
    Image16 img;
    if (f==1) {
      img = getImageAt(0);
    } else {
      switch (settings.layer(f-1).type()) {
      case Layer::Type::Clone:
      case Layer::Type::Inpaint:
        img = applyInpaint(imgF2, settings.layer(f-1));
        break;
      default: 
        imgF1 = getImageAt(f-1);
        img = applyMask(imgF1, imgF2, settings.layer(f-1));
        break;
      }
    }
    //imgF1.toQImage().save(QString("/tmp/image-%1-1.jpg").arg(F));
    //imgF2.toQImage().save(QString("/tmp/image-%1-2.jpg").arg(F));
    //img.toQImage().save(QString("/tmp/image-%1.jpg").arg(F));
    if (f>N) {
      pDebug() << "AA::retrieveReduced returning layered img sized "
	       << img.size();
      return img;
    } else {
      layerAdjuster(f)->setOriginal(img);
      validInputUntil = f;
      imgF2 = img;
      f++;
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

