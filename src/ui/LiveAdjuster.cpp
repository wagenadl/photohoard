// LiveAdjuster.cpp

#include "LiveAdjuster.h"
#include "Sliders.h"
#include "AllControls.h"
#include "InterruptableAdjuster.h"
#include "Adjuster.h"
#include "OriginalFinder.h"
#include "AutoCache.h"

LiveAdjuster::LiveAdjuster(PhotoDB const &db, 
                           class AllControls *controls,
                           class AutoCache *cache,
                           QObject *parent):
  QObject(parent), db(db), controls(controls), cache(cache) {
  ofinder = new OriginalFinder(db, this);
  adj = new Adjuster(this);
  adjuster = new InterruptableAdjuster(adj, this);
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
}

void LiveAdjuster::requestAdjusted(quint64 v, QSize s) {
  bool newvsn = version!=v;
  mustshowupdate = true;
  if (newvsn) {
    QString mods = db.simpleQuery("select mods from versions"
                                  " where id=:a limit 1", v).toString();
    sliders.setAll(mods);
    controls->setQuietly(sliders);
  }
  qDebug() << "requestAdjusted" << v << s << newvsn
           << adjuster->maxAvailableSize();
  if (newvsn || PSize(s).exceeds(adjuster->maxAvailableSize())) {
    if (newvsn)
      adjuster->clear();
    else
      adjuster->cancelRequest();
    ofinder->requestScaledOriginal(version = v, targetsize = s);
  } else {  
    adjuster->requestReduced(sliders, targetsize = s);
  }
}

void LiveAdjuster::setSlider(QString k, double v) {
  qDebug() << "LiveAdjuster::setSlider" << k << v;
  if (v==sliders.get(k))
    return;
  mustshowupdate = true;
  mustoffermod = true;
  sliders.set(k, v);
  if (adjuster->isEmpty()) {
    //
  } else {
    if (targetsize.isEmpty())
      adjuster->requestFull(sliders);
    else
      adjuster->requestReduced(sliders, targetsize);
  }
  QString mods = sliders.getAll();
  db.simpleQuery("update versions set mods=:a where id==:b limit 1",
                 mods, version);
}

void LiveAdjuster::provideAdjusted(Image16 img) {
  qDebug() << "LiveAdjuster::provideAdjusted" << img.size();
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
  qDebug() << "LiveAdjuster::provideOriginal" << v << img.size();
  if (v!=version)
    return;
  adjuster->cancelRequest();
  adjuster->setOriginal(img);

  if (targetsize.isEmpty())
    adjuster->requestFull(sliders);
  else
    adjuster->requestReduced(sliders, targetsize);
}

void LiveAdjuster::provideScaledOriginal(quint64 v, QSize osize, Image16 img) {
  qDebug() << "LiveAdjuster::provideScaledOriginal" << v << img.size() << osize << version;
  if (v!=version)
    return;
  adjuster->setOriginal(img, osize);

  if (targetsize.isEmpty())
    adjuster->requestReduced(sliders, img.size());
  else
    adjuster->requestReduced(sliders, targetsize);
}
