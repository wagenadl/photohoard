// InterruptableAdjuster.h

#ifndef INTERRUPTABLEADJUSTER_H

#define INTERRUPTABLEADJUSTER_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include "Sliders.h"
#include "Image16.h"

class InterruptableAdjuster: public QThread {
  Q_OBJECT;
public:
  InterruptableAdjuster(class Adjuster *adjuster, QObject *parent=0);
  virtual ~InterruptableAdjuster();
  void requestFull(Sliders const &settings);
  void requestReduced(Sliders const &settings, PSize maxSize);
  void requestROI(Sliders const &settings, QRect roi);
  void requestReducedROI(Sliders const &settings, QRect roi, PSize maxSize);
  void cancelRequest();
  void clear();
  PSize maxAvailableSize();
  bool isEmpty();
  void setOriginal(Image16 img, PSize osize=PSize());
signals:
  void ready(Image16);
  // Note that "ready" does not imply success: the image can be null.
protected:
  void start();
  void stop();
  virtual void run();
private:
  void handleNewRequest();
  void handleNewImage();
private:
  Adjuster *adjuster;
  QMutex mutex;
  QWaitCondition waitcond;
  bool cancel, newreq, clear_;
  Sliders rqSliders;
  QRect rqRect;
  PSize rqSize;
  bool stopsoon;
  bool empty;
  PSize maxAvail;
  Image16 newOriginal;
  PSize oSize;
};

#endif
