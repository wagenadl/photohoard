// LiveAdjuster.cpp

#include "LiveAdjuster.h"
#include "Sliders.h"
#include "AutoCache.h"
#include "AllControls.h"
#include "InterruptableAdjuster.h"
#include "Adjuster.h"
#include "OriginalFinder.h"

LiveAdjuster::LiveAdjuster(PhotoDB const &db, class AutoCache *cache,
                           class AllControls *controls,
                           QObject *parent):
  QObject(parent), db(db), cache(cache), controls(controls) {
  ofinder = new OriginalFinder(db, cache->basicCache(), this);
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
  mustupdate = false;
}

void LiveAdjuster::setTargetSize(QSize s) {
  if (version>0 && !targetsize.isEmpty()
      && (s.width()>targetsize.width() || s.height()>targetsize.height())) {
    targetsize = s;
    adjuster->cancelRequest();
    ofinder->requestScaledOriginal(version, targetsize);
  } else {
    targetsize = s;
  }
}

void LiveAdjuster::setTargetVersion(quint64 v) {
  if (version==v)
    return;
  version = v;
  adjuster->cancelRequest();
  if (targetsize.isEmpty())
    ofinder->requestOriginal(version);
  else
    ofinder->requestScaledOriginal(version, targetsize);
  /* But do we really need to request the full size? And do we even need
     to request it always? Probably not. */

  mustupdate = false;
  QSqlQuery q = db.query("select photo, mods from versions"
                         " where id=:a limit 1", version);
  if (!q.next()) {
    qDebug() << "LiveAdjuster: version not found";
    version = 0;
    return;
  }

  sliders.setAll(q.value(1).toString());
  QMap<QString, double> values;
  for (auto k: sliders.keys())
    values[k] = sliders.get(k);
  controls->setQuietly(values);
}

void LiveAdjuster::setSlider(QString k, double v) {
  sliders.set(k, v);
  if (adj->isEmpty()) {
    mustupdate = true;
  } else {
    if (targetsize.isEmpty())
      adjuster->requestFull(sliders);
    else
      adjuster->requestReduced(sliders, targetsize);
  }
}

void LiveAdjuster::provideAdjusted(Image16 img) {
  img.convertTo(Image16::Format::sRGB8);
  emit imageChanged(version, img.size(), img);
}
  
void LiveAdjuster::provideOriginal(quint64 v, Image16 img) {
  if (v!=version)
    return;
  adjuster->cancelRequest();
  adj->setOriginal(img);
  if (mustupdate) {
    mustupdate = false;
    if (targetsize.isEmpty())
      adjuster->requestFull(sliders);
    else
      adjuster->requestReduced(sliders, targetsize);
  }
}

void LiveAdjuster::provideScaledOriginal(quint64 v, QSize osize, Image16 img) {
  if (v!=version)
    return;
  adj->setReduced(img, osize);
  if (mustupdate) {
    mustupdate = false;
    if (targetsize.isEmpty())
      adjuster->requestReduced(sliders, img.size());
    else
      adjuster->requestReduced(sliders, targetsize);
  }
}
