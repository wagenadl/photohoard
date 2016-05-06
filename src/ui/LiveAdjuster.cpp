// LiveAdjuster.cpp

#include "LiveAdjuster.h"
#include "Adjustments.h"
#include "InterruptableAdjuster.h"
#include "Adjuster.h"
#include "OriginalFinder.h"
#include "AutoCache.h"
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
    sliders.readFromDB(v, *db);
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

  PSize maxav = adjuster->maxAvailableSize(sliders, v);
  bool needBigger = PSize(s).exceeds(maxav);
  bool canBigger = originalSize.isEmpty()
    || Adjuster::mapCropSize(originalSize, sliders, originalSize)
    .exceeds(maxav);
  
  if (newvsn || (needBigger && canBigger)) {
    if (newvsn)
      adjuster->clear();
    else
      adjuster->cancelRequest();
    PSize needed = Adjuster::neededScaledOriginalSize(originalSize, sliders,
						      targetsize);
    ofinder->requestScaledOriginal(version, needed);
  } else {
    adjuster->requestReduced(sliders, s, version);
  }
}

void LiveAdjuster::reloadSliders(quint64 v, Adjustments sli) {
  if (v!=version) {
    pDebug() << "LiveAdjuster::reloadSliders: vsn mismatch";
    return;
  }
  if (sli==sliders)
    return;
  
  mustshowupdate = true;
  mustoffermod = true;
  if (adjuster->isEmpty()) {
    ofinder->requestScaledOriginal(version, targetsize);
  } else {
    if (targetsize.isEmpty())
      adjuster->requestFull(sli, version);
    else
      adjuster->requestReduced(sli, targetsize, version);
  }

  Untransaction t(db);
  for (QString k: Adjustments::keys()) {
    double v = sli.get(k);
    if (v != sliders.get(k)) {
      QSqlQuery q = db->query("select v from adjustments"
                              " where version==:a and k==:b", version, k);
      if (q.next()) 
        db->addUndoStep(version, k, q.value(0), v);
      else
        db->addUndoStep(version, k, QVariant(), v);
      if (v==Adjustments::defaultFor(k))
        db->query("delete from adjustments where version==:a and k==:b",
                  version, k);
      else
        db->query("insert or replace into adjustments (version, k, v)"
                  " values (:a, :b, :c)", version, k, v);
    }
  }

  sliders = sli;
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
    adjuster->requestFull(sliders, v);
  else
    adjuster->requestReduced(sliders, targetsize, v);
}

void LiveAdjuster::provideScaledOriginal(quint64 v, QSize osize, Image16 img) {
  if (v!=version)
    return;
  originalSize = osize;
  adjuster->setReduced(img, osize, v);

  if (targetsize.isEmpty())
    adjuster->requestReduced(sliders, img.size(), v);
  else
    adjuster->requestReduced(sliders, targetsize, v);
}
