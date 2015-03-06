// InterruptableAdjuster.cpp

#include "InterruptableAdjuster.h"
#include "Adjuster.h"
#include <QMutexLocker>

InterruptableAdjuster::InterruptableAdjuster(Adjuster *adjuster,
                                             QObject *parent):
  QThread(parent), adjuster(adjuster) {
  emit_while_locked = false;
  start();
}

InterruptableAdjuster::~InterruptableAdjuster() {
  stop();
  if (!wait(1000)) {
    qDebug() << "InterruptableAdjuster: Failed to stop thread; terminating.";
    terminate();
    if (!wait(1000))
      qDebug() << "InterruptableAdjuster: Still failed to stop thread";
  }
}

void InterruptableAdjuster::requestFull(Sliders const &settings) {
  requestReducedROI(settings, QRect(), PSize());
}

void InterruptableAdjuster::requestReduced(Sliders const &settings,
                                           PSize maxSize) {
  requestReducedROI(settings, QRect(), maxSize);
}
  
void InterruptableAdjuster::requestROI(Sliders const &settings, QRect roi) {
  requestReducedROI(settings, roi, PSize());
}

void InterruptableAdjuster::requestReducedROI(Sliders const &settings,
                                              QRect roi, PSize maxSize) {
  QMutexLocker l(&mutex);
  adjuster->cancel();
  newreq = true;
  cancel = false;
  rqSliders = settings;
  rqRect = roi;
  rqSize = maxSize;
  waitcond.wakeOne();
}

void InterruptableAdjuster::cancelRequest() {
  QMutexLocker l(&mutex);
  adjuster->cancel();
  newreq = false;
  cancel = true;
}  

void InterruptableAdjuster::start() {
  stopsoon = false;
  newreq = false;
  QThread::start();
}

void InterruptableAdjuster::stop() {
  stopsoon = true;
}

void InterruptableAdjuster::run() {
  mutex.lock();
  while (!stopsoon) {
    if (newreq) {
      QRect r = rqRect;
      PSize s = rqSize;
      Sliders sli = rqSliders;
      newreq = false;
      cancel = false;
      mutex.unlock();
      Image16 img;
      if (r.isEmpty()) {
        if (s.isEmpty()) 
          img = adjuster->retrieveFull(sli);
        else
          img = adjuster->retrieveReduced(sli, s);
      } else {
        if (s.isEmpty())
          img = adjuster->retrieveROI(sli, r);
        else
          img = adjuster->retrieveReducedROI(sli, r, s);
      }
      mutex.lock();
      if (cancel || newreq) {
        cancel = false;
      } else if (emit_while_locked) {
        emit ready(img);
      } else {
        mutex.unlock();
        emit ready(img);
        mutex.lock();
      }
    } else {
      waitcond.wait(&mutex);
    }
  }
  mutex.unlock();
}

void InterruptableAdjuster::emitWhileLocked() {
  QMutexLocker l(&mutex);
  emit_while_locked = true;
}

void InterruptableAdjuster::emitWhileUnlocked() {
  QMutexLocker l(&mutex);
  emit_while_locked = false;
}

