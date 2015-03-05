// LiveAdjuster.h

#ifndef LIVEADJUSTER_H

#define LIVEADJUSTER_H

#include <QObject>
#include "PhotoDB.h"
#include "Image16.h"
#include "Sliders.h"

class LiveAdjuster: public QObject {
  Q_OBJECT;
public:
  LiveAdjuster(PhotoDB const &db, class AutoCache *cache,
               class AllControls *controls,
               QObject *parent=0);
public slots:
  void setTargetVersion(quint64 version);
  void setTargetSize(QSize);
signals:
  void imageChanged(quint64, QSize, Image16);
  // The size is the original size
private slots:
  void setSlider(QString, double);
  void provideOriginal(quint64, Image16);
  void provideScaledOriginal(quint64, QSize osize, Image16);
  void provideAdjusted(Image16);
private:
  PhotoDB db;
  AutoCache *cache; // we do not own
  AllControls *controls; // we do not own
  quint64 version;
  QSize targetsize;
  Sliders sliders;
  class Adjuster *adj; // we own
  class InterruptableAdjuster *adjuster; // we own
  class OriginalFinder *ofinder; // we own
  bool mustupdate;
};

#endif
