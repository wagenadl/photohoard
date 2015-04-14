// LiveAdjuster.cpp

#include "LiveAdjuster.h"
#include "Sliders.h"
#include "AllControls.h"
#include "InterruptableAdjuster.h"
#include "Adjuster.h"
#include "OriginalFinder.h"
#include "AutoCache.h"
#include "PDebug.h"

LiveAdjuster::LiveAdjuster(PhotoDB *db, 
                           class AllControls *controls,
                           class AutoCache *cache,
                           QObject *parent):
  QObject(parent), db(db), controls(controls), cache(cache) {
  ofinder = new OriginalFinder(db, this);
  adjuster = new InterruptableAdjuster(this);
  connect(adjuster, SIGNAL(ready(Image16)),
          SLOT(provideAdjusted(Image16)));
  connect(controls, SIGNAL(valueChanged(QString, double)),
          SLOT(setSlider(QString, double)));
  connect(ofinder, SIGNAL(originalAvailable(quint64, Image16)),
          SLOT(provideOriginal(quint64, Image16)));
  connect(ofinder, SIGNAL(scaledOriginalAvailable(quint64, QSize, Image16)),
          SLOT(provideScaledOriginal(quint64, QSize, Image16)));
  version = 0;
  mustoffermod = false;
  mustshowupdate = false;
  controls->setEnabled(false);
}

void LiveAdjuster::clear() {
  version = 0;
  adjuster->clear();
  controls->setEnabled(false);
}

void LiveAdjuster::markVersionAndSize(quint64 v, QSize s) {
  bool newvsn = version!=v;
  mustshowupdate = true;
  if (newvsn) {
    originalSize = ofinder->originalSize(v);
    //    pDebug() << "LiveAdjuster newvsn size is " << originalSize ;
    sliders.reset();
    QSqlQuery q
      = db->constQuery("select k,v from adjustments where version==:a", v);
    while (q.next())
      sliders.set(q.value(0).toString(), q.value(1).toDouble());
    q.finish();
    controls->setQuietly(sliders);
    controls->setEnabled(true);
  }
  targetsize = s;
  version = v;

  if (newvsn) {
    adjuster->clear();
  }
}

void LiveAdjuster::requestAdjusted(quint64 v, QSize s) {
  bool newvsn = version!=v;
  if (newvsn)
    markVersionAndSize(v, s);
  else
    targetsize = s;
  mustshowupdate = true;

  PSize maxav = adjuster->maxAvailableSize(sliders);
  bool needBigger = PSize(s).exceeds(maxav);
  bool canBigger = originalSize.isEmpty()
    || Adjuster::mapCropSize(originalSize, sliders, originalSize)
    .exceeds(maxav);
  //  pDebug() << "LiveAdjuster::requestAdjusted" << "o=" << originalSize << " av=" << maxav
//	   << "desired=" << s << " => "
//	   << needBigger << canBigger
//	   << Adjuster::mapCropSize(originalSize, sliders, originalSize);
  
  if (newvsn || (needBigger && canBigger)) {
    if (newvsn)
      adjuster->clear();
    else
      adjuster->cancelRequest();
    PSize needed = Adjuster::neededScaledOriginalSize(originalSize, sliders,
						      targetsize);
    ofinder->requestScaledOriginal(version, needed);
  } else {
    adjuster->requestReduced(sliders, s);
  }
}

void LiveAdjuster::setSlider(QString k, double v) {
  if (v==sliders.get(k))
    return;
  
  mustshowupdate = true;
  mustoffermod = true;
  sliders.set(k, v);

  if (adjuster->isEmpty()) {
    ofinder->requestScaledOriginal(version, targetsize);
  } else {
    if (targetsize.isEmpty())
      adjuster->requestFull(sliders);
    else
      adjuster->requestReduced(sliders, targetsize);
  }

  db->query("insert or replace into adjustments (version, k, v)"
            " values (:a, :b, :c)", version, k, v);
}

void LiveAdjuster::provideAdjusted(Image16 img) {
  if (mustoffermod) {
    mustoffermod = false;
    cache->cacheModified(version, img);
  }
  if (mustshowupdate) {
    mustshowupdate = false;
    img.convertTo(Image16::Format::sRGB8);
    emit imageChanged(img, version);
  }
}
  
void LiveAdjuster::provideOriginal(quint64 v, Image16 img) {
  //  pDebug() << "LiveAdjuster::provideOriginal" << v << img.size();
  if (v!=version)
    return;
  originalSize = img.size();
  adjuster->cancelRequest();
  adjuster->setOriginal(img);

  if (targetsize.isEmpty())
    adjuster->requestFull(sliders);
  else
    adjuster->requestReduced(sliders, targetsize);
}

void LiveAdjuster::provideScaledOriginal(quint64 v, QSize osize, Image16 img) {
  //  pDebug() << "LiveAdjuster: provideScaledOriginal" << v << osize << img.size();
  if (v!=version)
    return;
  originalSize = osize;
  adjuster->setOriginal(img, osize);

  if (targetsize.isEmpty())
    adjuster->requestReduced(sliders, img.size());
  else
    adjuster->requestReduced(sliders, targetsize);
}
