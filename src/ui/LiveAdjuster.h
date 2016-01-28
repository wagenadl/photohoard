// LiveAdjuster.h

#ifndef LIVEADJUSTER_H

#define LIVEADJUSTER_H

#include <QObject>
#include "PhotoDB.h"
#include "Image16.h"
#include "Adjustments.h"

class LiveAdjuster: public QObject {
  Q_OBJECT;
public:
  LiveAdjuster(PhotoDB *db, 
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
  void imageAvailable(Image16 img, quint64 version);
  /* In addition, when a new adjustment is made, the image is offered
     to the autocache through CACHEMODIFIED, which will immediately cause an
     AVAILABLE signal to be emitted with a non-zero CHGID. That happens
     before this signal is emitted.
  */
private slots:
  void setSlider(QString, double);
  void provideOriginal(quint64, Image16);
  void provideScaledOriginal(quint64, QSize osize, Image16);
  void provideAdjusted(Image16);
private:
  PhotoDB *db;
  AllControls *controls; // we do not own
  AutoCache *cache;  // we do not own
  quint64 version;
  PSize targetsize;
  Adjustments sliders;
  class Adjuster *adj; // we own
  class InterruptableAdjuster *adjuster; // we own
  class OriginalFinder *ofinder; // we own
  bool mustshowupdate;
  bool mustoffermod;
  PSize originalSize;
};

#endif
