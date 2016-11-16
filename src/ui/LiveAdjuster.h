// LiveAdjuster.h

#ifndef LIVEADJUSTER_H

#define LIVEADJUSTER_H

#include <QObject>
#include "PhotoDB.h"
#include "Image16.h"
#include "Adjustments.h"
#include <QMap>

class LiveAdjuster: public QObject {
  Q_OBJECT;
public:
  LiveAdjuster(PhotoDB *db, 
               class AutoCache *cache,
               QObject *parent=0);
public slots:
  void clear();
  void requestAdjusted(quint64 version, QSize size);
  void markVersionAndSize(quint64 version, QSize size);
  /* The latter doesn't immediately request an image, but prepares for
     later setSlider calls that will need an image. */
  void reloadSliders(quint64 vsn, int layer, Adjustments a);
  void reloadLayers(quint64 vsn, int lowest);
signals:
  void imageAvailable(Image16 img, quint64 version, QSize fullsize);
  /* In addition, when a new adjustment is made, the image is offered
     to the autocache through CACHEMODIFIED, which will immediately cause an
     AVAILABLE signal to be emitted with a non-zero CHGID. That happens
     before this signal is emitted.
  */
private slots:
  void provideOriginal(quint64, Image16);
  void provideScaledOriginal(quint64, QSize osize, Image16);
  void provideAdjusted(Image16, quint64 v, QSize fullsize);
private:
  void forceUpdate();
  void loadLayers(int lowest);
private:
  PhotoDB *db;
  AutoCache *cache;  // we do not own
  quint64 version;
  PSize targetsize;
  QMap<int, Adjustments> adjs;
  class InterruptableAdjuster *adjuster; // we own
  class OriginalFinder *ofinder; // we own
  bool mustshowupdate;
  bool mustoffermod;
  PSize originalSize;
};

#endif
