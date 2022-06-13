// LiveAdjuster.cpp

#include "LiveAdjuster.h"
#include "Adjustments.h"
#include "InterruptableAdjuster.h"
#include "Adjuster.h"
#include "OriginalFinder.h"
#include "AutoCache.h"
#include "Layers.h"
#include "Geometry.h"
#include "PDebug.h"
#include "ImgAvg.h"

LiveAdjuster::LiveAdjuster(PhotoDB *db, 
                           class AutoCache *cache,
                           QObject *parent):
  QObject(parent), db(db), cache(cache) {
  ofinder = new OriginalFinder(db, this);
  adjuster = new InterruptableAdjuster(this);
  connect(adjuster, SIGNAL(ready(Image16, quint64, QSize)),
          SLOT(provideAdjusted(Image16, quint64, QSize)));
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
  targetsize = s;
  version = v;

  if (newvsn) {
    originalSize = db->originalSize(v);
    loadLayers(0);
    adjuster->clear();
  }
}

void LiveAdjuster::loadLayers(int lowest) {
  adjs.rereadFromDB(version, lowest, *db);
}  

void LiveAdjuster::requestAdjusted(quint64 v, QSize s) {
  bool newvsn = version!=v;
  if (newvsn)
    markVersionAndSize(v, s);
  else
    targetsize = s;
  mustshowupdate = true;

  PSize maxav = adjuster->maxAvailableSize(adjs.baseAdjustments(), v);
  bool needBigger = PSize(s).exceeds(maxav);
  bool canBigger = originalSize.isEmpty()
    || Geometry::croppedSize(originalSize, adjs.baseAdjustments())
    .exceeds(maxav);
  
  if (newvsn || (needBigger && canBigger)) {
    if (newvsn)
      adjuster->clear();
    else
      adjuster->cancelRequest();
    PSize needed = Geometry::neededScaledOriginalSize(originalSize,
						      adjs.baseAdjustments(),
						      targetsize);
    ofinder->requestScaledOriginal(version, needed);
  } else {
    adjuster->requestReduced(adjs, s, version);
  }
}

void LiveAdjuster::reloadLayers(quint64 v, int lowest) {
  if (v!=version) {
    COMPLAIN("LiveAdjuster::reloadLayers: vsn mismatch");
    return;
  }
  //  pDebug() << "reloadLayers" << v << lowest;
  loadLayers(lowest);
  forceUpdate();
}

void LiveAdjuster::forceUpdate() {
  mustshowupdate = true;
  mustoffermod = true;
  //  pDebug() << "LiveAdjuster::forceUpdate" << adjuster->isEmpty()
  //	   << targetsize.isEmpty();
  if (adjuster->isEmpty()) {
    ofinder->requestScaledOriginal(version, targetsize);
  } else {
    if (targetsize.isEmpty())
      adjuster->requestFull(adjs, version);
    else
      adjuster->requestReduced(adjs, targetsize, version);
  }
}  

void LiveAdjuster::reloadSliders(quint64 v, int lay, Adjustments sli) {
  if (v!=version) {
    COMPLAIN("LiveAdjuster::reloadSliders: vsn mismatch");
    return;
  }
  //  pDebug() << "LiveAdjuster::reloadSliders" << v << lay << sli;
  if (lay==0) {
    if (sli==adjs.baseAdjustments())
      return;
    adjs.baseAdjustments() = sli;
  } else {
    ASSERT(lay>=1 && lay<=adjs.layerCount());
    if (sli==adjs.layerAdjustments(lay))
      return;
    adjs.layerAdjustments(lay) = sli;
  }

  forceUpdate();
}

void LiveAdjuster::provideAdjusted(Image16 img, quint64 v, QSize fs) {
  //  pDebug() << "LiveAdjuster::provideAdjusted" << img.size() << fs << averagePixel(img);
  if (v!=version) {
    COMPLAIN("LiveAdjuster: version mismatch");
    return;
  }
  if (mustoffermod) {
    //    pDebug() << "LiveAdj: must offer mod";
    mustoffermod = false;
    cache->cacheModified(version, img);
  } else {
    //    pDebug() << "LiveAdj: not offering mod";
  }
  if (mustshowupdate) {
    //    pDebug() << "LiveAdj: mustshowupdate";
    mustshowupdate = false;
    img.convertTo(Image16::Format::sRGB8);
    //    pDebug() << "LiveAdj::available" << version
    //	     << img.size() << averagePixel(img);
    emit imageAvailable(img, version, fs);
  } else {
    //    pDebug() << "LiveAdj: not showing update";
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
