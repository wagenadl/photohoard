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
  LiveAdjuster(PhotoDB const &db, 
               class AllControls *controls,
               class AutoCache *cache,
               QObject *parent=0);
public slots:
  void clear();
  void requestAdjusted(quint64 version, QSize size);
  void markVersionAndSize(quint64 version, QSize size);
  /* The latter doesn't immediately request an image, but prepares for
     later setSlider calls that will need an image. */
signals:
  void imageChanged(Image16 img, quint64 version);
private slots:
  void setSlider(QString, double);
  void provideOriginal(quint64, Image16);
  void provideScaledOriginal(quint64, QSize osize, Image16);
  void provideAdjusted(Image16);
private:
  PhotoDB db;
  AllControls *controls; // we do not own
  AutoCache *cache;  // we do not own
  quint64 version;
  PSize targetsize;
  Sliders sliders;
  class Adjuster *adj; // we own
  class InterruptableAdjuster *adjuster; // we own
  class OriginalFinder *ofinder; // we own
  bool mustshowupdate;
  bool mustoffermod;
  PSize originalSize;
};

#endif
