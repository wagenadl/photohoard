// LiveAdjuster.cpp

#include "LiveAdjuster.h"
#include "Sliders.h"
#include "AllControls.h"
#include "InterruptableAdjuster.h"
#include "Adjuster.h"
#include "OriginalFinder.h"

LiveAdjuster::LiveAdjuster(PhotoDB const &db, 
                           class AllControls *controls,
                           QObject *parent):
  QObject(parent), db(db), controls(controls) {
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
}

void LiveAdjuster::requestAdjusted(quint64 v, QSize s) {
  bool newvsn = version!=v;
  if (newvsn) {
    QString mods = db.simpleQuery("select mods from versions"
                                  " where id=:a limit 1", v).toString();
    sliders.setAll(mods);
    controls->setQuietly(sliders);
  }
  qDebug() << "requestAdjusted" << v << s << newvsn << adj->maxAvailableSize();
  if (newvsn || PSize(s).exceeds(adj->maxAvailableSize())) {
    adjuster->cancelRequest();
    ofinder->requestScaledOriginal(version = v, targetsize = s);
  } else {  
    adjuster->requestReduced(sliders, targetsize = s);
  }
}

void LiveAdjuster::setSlider(QString k, double v) {
  qDebug() << "LiveAdjuster::setSlider" << k << v;
  sliders.set(k, v);
  if (adj->isEmpty()) {
    //
  } else {
    if (targetsize.isEmpty())
      adjuster->requestFull(sliders);
    else
      adjuster->requestReduced(sliders, targetsize);
  }
}

void LiveAdjuster::provideAdjusted(Image16 img) {
  qDebug() << "LiveAdjuster::provideAdjusted" << img.size();
  img.convertTo(Image16::Format::sRGB8);
  emit imageChanged(img, version);
}
  
void LiveAdjuster::provideOriginal(quint64 v, Image16 img) {
  qDebug() << "LiveAdjuster::provideOriginal" << v << img.size();
  if (v!=version)
    return;
  adjuster->cancelRequest();
  adj->setOriginal(img);

  if (targetsize.isEmpty())
    adjuster->requestFull(sliders);
  else
    adjuster->requestReduced(sliders, targetsize);
}

void LiveAdjuster::provideScaledOriginal(quint64 v, QSize osize, Image16 img) {
  qDebug() << "LiveAdjuster::provideScaledOriginal" << v << img.size() << osize;
  if (v!=version)
    return;
  adj->setReduced(img, osize);

  if (targetsize.isEmpty())
    adjuster->requestReduced(sliders, img.size());
  else
    adjuster->requestReduced(sliders, targetsize);
}
