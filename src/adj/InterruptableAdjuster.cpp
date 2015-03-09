// InterruptableAdjuster.cpp

#include "InterruptableAdjuster.h"
#include "Adjuster.h"
#include <QMutexLocker>

InterruptableAdjuster::InterruptableAdjuster(Adjuster *adjuster,
                                             QObject *parent):
  QThread(parent), adjuster(adjuster) {
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

PSize InterruptableAdjuster::maxAvailableSize() {
  QMutexLocker l(&mutex);
  return maxAvail;
}

bool InterruptableAdjuster::isEmpty() {
  QMutexLocker l(&mutex);
  return empty;
}

void InterruptableAdjuster::setOriginal(Image16 img, PSize siz) {
  QMutexLocker l(&mutex);
  newOriginal = img;
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
  qDebug() << "IA: gotImage" << img.size() << cancel << newreq << clear_;
  mutex.lock();
  if (cancel || newreq || clear_) {
    cancel = false;
    if (clear_) {
      adjuster->clear();
      clear_ = false;
    }
  } else {
    mutex.unlock();
    emit ready(img);
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
    maxAvail = adjuster->maxAvailableSize();
    empty = adjuster->isEmpty();
    if (clear_) {
      adjuster->clear();
      clear_ = false;
    } else if (cancel) {
      cancel = false;
    } else if (!newOriginal.isNull()) {
      qDebug() << "IA: newOriginal";
      handleNewImage();
    } else if (newreq) {
      qDebug() << "IA: newRequest";
      handleNewRequest();
    } else {
      qDebug() << "IA: Waiting";
      waitcond.wait(&mutex);
    }
  }
  mutex.unlock();
}


