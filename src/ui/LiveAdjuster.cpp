// LiveAdjuster.cpp

#include "LiveAdjuster.h"
#include "Sliders.h"
#include "AutoCache.h"
#include "AllControls.h"
#include "Adjuster.h"

LiveAdjuster::LiveAdjuster(PhotoDB const &db, class AutoCache *cache,
                           class AllControls *controls,
                           QObject *parent):
  QObject(parent), db(db), cache(cache), controls(controls) {
  adjuster = new Adjuster(this);
  connect(cache, SIGNAL(originalAvailable(quint64, Image16)),
          SLOT(provideOriginal(quint64, Image16)));
  connect(cache, SIGNAL(scaledOriginalAvailable(quint64, QSize, Image16)),
          SLOT(provideScaledOriginal(quint64, QSize, Image16)));
  connect(controls, SIGNAL(valueChanged(QString, double)),
          SLOT(setSlider(QString, double)));
  version = 0;
  mustupdate = false;
}

void LiveAdjuster::setTargetVersion(quint64 v) {
  if (version==v)
    return;
  version = v;
  adjuster->clear();
  cache->requestOriginal(version); // do this early, so thread can get started
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
  if (adjuster->isEmpty()) {
    mustupdate = true;
  } else {
    Image16 img = adjuster->retrieveFull(sliders);
    img.convertTo(Image16::Format::sRGB8);
    emit imageChanged(version, img.size(), img);
  }
}
  
void LiveAdjuster::provideOriginal(quint64 v, Image16 img) {
  if (v!=version)
    return;
  adjuster->setOriginal(img);
  if (mustupdate) {
    mustupdate = false;
    Image16 img = adjuster->retrieveFull(sliders);
    img.convertTo(Image16::Format::sRGB8);
    emit imageChanged(version, img.size(), img);
  }
}

void LiveAdjuster::provideScaledOriginal(quint64 v, QSize s, Image16 img) {
  if (v!=version)
    return;
  QSize ms = adjuster->maxAvailableSize();
  if (img.width()*img.height() < ms.width()*ms.height())
    return;
  adjuster->setReduced(img, s);
  if (mustupdate) {
    mustupdate = false;
    Image16 img = adjuster->retrieveReduced(sliders, img.size());
    img.convertTo(Image16::Format::sRGB8);
    emit imageChanged(version, s, img);
  }
}
