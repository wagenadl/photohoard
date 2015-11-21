// InterruptableAdjuster.cpp

#include "InterruptableAdjuster.h"
#include "Adjuster.h"
#include <QMutexLocker>
#include "PDebug.h"

InterruptableAdjuster::InterruptableAdjuster(QObject *parent):
  QThread(parent) {
  adjuster = new Adjuster(this);
  start();
}

InterruptableAdjuster::~InterruptableAdjuster() {
  stop();
  if (!wait(1000)) {
    pDebug() << "InterruptableAdjuster: Failed to stop thread; terminating.";
    terminate();
    if (!wait(1000))
      pDebug() << "InterruptableAdjuster: Still failed to stop thread";
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

void InterruptableAdjuster::clear() {
  QMutexLocker l(&mutex);
  adjuster->cancel();
  newreq = false;
  cancel = true;
  clear_ = true;
}

void InterruptableAdjuster::start() {
  stopsoon = false;
  newreq = false;
  cancel = false;
  clear_ = false;
  empty = true;
  QThread::start();
}

void InterruptableAdjuster::stop() {
  stopsoon = true;
}

PSize InterruptableAdjuster::maxAvailableSize(Sliders const &s) {
  if (clear_)
    return PSize();
  else
    return Adjuster::mapCropSize(oSize, s, scaledOSize);
}

bool InterruptableAdjuster::isEmpty() {
  QMutexLocker l(&mutex);
  return empty;
}

void InterruptableAdjuster::setOriginal(Image16 img) {
  setReduced(img, PSize());
}

void InterruptableAdjuster::setReduced(Image16 img, PSize siz) {
  QMutexLocker l(&mutex);
  newOriginal = img;
  scaledOSize = img.size();
  oSize = siz;
}    

void InterruptableAdjuster::handleNewRequest() {
  QRect r = rqRect;
  PSize s = rqSize;
  Sliders sli = rqSliders;
  newreq = false;
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
  if (cancel || newreq || clear_) {
    cancel = false;
    if (clear_) {
      adjuster->clear();
      clear_ = false;
    }
  } else {
    mutex.unlock();
    pDebug() << "InterruptableAdjuster: emitting ready";
    emit ready(img);
    pDebug() << "InterruptableAdjuster: emitted ready";
    mutex.lock();
  }
}  

void InterruptableAdjuster::handleNewImage() {
  if (oSize.isEmpty())
    adjuster->setOriginal(newOriginal);
  else
    adjuster->setReduced(newOriginal, oSize);
  newOriginal = Image16();
}

void InterruptableAdjuster::run() {
  mutex.lock();
  while (!stopsoon) {
    empty = adjuster->isEmpty();
    if (clear_) {
      adjuster->clear();
      clear_ = false;
      cancel = false;
    } else if (cancel) {
      cancel = false;
    } else if (!newOriginal.isNull()) {
      pDebug() << "InterruptableAdjuster: new image";
      handleNewImage();
    } else if (newreq) {
      pDebug() << "InterruptableAdjuster: new request";
      handleNewRequest();
    } else {
      waitcond.wait(&mutex);
      pDebug() <<"InterruptableAdjuster: woke up";
    }
  }
  mutex.unlock();
}


