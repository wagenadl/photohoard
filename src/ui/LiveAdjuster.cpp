// LiveAdjuster.cpp

#include "LiveAdjuster.h"
#include "Adjustments.h"
#include "InterruptableAdjuster.h"
#include "Adjuster.h"
#include "OriginalFinder.h"
#include "AutoCache.h"
#include "Layers.h"
#include "PDebug.h"

LiveAdjuster::LiveAdjuster(PhotoDB *db, 
                           class AutoCache *cache,
                           QObject *parent):
  QObject(parent), db(db), cache(cache) {
  ofinder = new OriginalFinder(db, this);
  adjuster = new InterruptableAdjuster(this);
  connect(adjuster, SIGNAL(ready(Image16, quint64)),
          SLOT(provideAdjusted(Image16, quint64)));
  connect(ofinder, SIGNAL(originalAvailable(quint64, Image16)),
          SLOT(provideOriginal(quint64, Image16)));
  connect(ofinder, SIGNAL(scaledOriginalAvailable(quint64, QSize, Image16)),
          SLOT(provideScaledOriginal(quint64, QSize, Image16)));
  version = 0;
  mustoffermod = false;
  mustshowupdate = false;
}

void LiveAdjuster::clear() {
  version = 0;
  adjuster->clear();
}

void LiveAdjuster::markVersionAndSize(quint64 v, QSize s) {
  bool newvsn = version!=v;
  mustshowupdate = true;
  if (newvsn) {
    originalSize = db->originalSize(v);
    Layers ll(v, db);
    int N = ll.count();
    adjs.clear();
    adjs[0] = Adjustments::fromDB(v, *db);
    for (int n=1; n<=N; n++)
      adjs[n] = Adjustments::fromDB(v, n, *db);
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

  PSize maxav = adjuster->maxAvailableSize(adjs[0], v);
  bool needBigger = PSize(s).exceeds(maxav);
  bool canBigger = originalSize.isEmpty()
    || Adjuster::mapCropSize(originalSize, adjs[0], originalSize)
    .exceeds(maxav);
  
  if (newvsn || (needBigger && canBigger)) {
    if (newvsn)
      adjuster->clear();
    else
      adjuster->cancelRequest();
    PSize needed = Adjuster::neededScaledOriginalSize(originalSize, adjs[0],
						      targetsize);
    ofinder->requestScaledOriginal(version, needed);
  } else {
    adjuster->requestReduced(adjs, s, version);
  }
}

void LiveAdjuster::reloadSliders(quint64 v, int lay, Adjustments sli) {
  if (v!=version) {
    pDebug() << "LiveAdjuster::reloadSliders: vsn mismatch";
    return;
  }
  ASSERT(adjs.contains(lay));
  if (sli==adjs[lay])
    return;

  adjs[lay] = sli;
  
  mustshowupdate = true;
  mustoffermod = true;
  if (adjuster->isEmpty()) {
    ofinder->requestScaledOriginal(version, targetsize);
  } else {
    if (targetsize.isEmpty())
      adjuster->requestFull(adjs, version);
    else
      adjuster->requestReduced(adjs, targetsize, version);
  }
}

void LiveAdjuster::provideAdjusted(Image16 img, quint64 v) {
  if (v!=version) {
    pDebug() << "LiveAdjuster: version mismatch";
    return;
  }
  if (mustoffermod) {
    mustoffermod = false;
    cache->cacheModified(version, img);
  } 
  if (mustshowupdate) {
    mustshowupdate = false;
    img.convertTo(Image16::Format::sRGB8);
    emit imageAvailable(img, version);
  }
}
  
void LiveAdjuster::provideOriginal(quint64 v, Image16 img) {
  if (v!=version)
    return;
  originalSize = img.size();
  adjuster->cancelRequest();
  adjuster->setOriginal(img, v);

  if (targetsize.isEmpty())
    adjuster->requestFull(adjs, v);
  else
    adjuster->requestReduced(adjs, targetsize, v);
}

void LiveAdjuster::provideScaledOriginal(quint64 v, QSize osize, Image16 img) {
  if (v!=version)
    return;
  originalSize = osize;
  adjuster->setReduced(img, osize, v);

  if (targetsize.isEmpty())
    adjuster->requestReduced(adjs, img.size(), v);
  else
    adjuster->requestReduced(adjs, targetsize, v);
}
