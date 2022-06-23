// InterruptableAdjuster.cpp

#include "InterruptableAdjuster.h"
#include "AllAdjuster.h"
#include <QMutexLocker>
#include "PDebug.h"
#include "Geometry.h"

InterruptableAdjuster::InterruptableAdjuster(QObject *parent):
  QThread(parent) {
  adjuster = new AllAdjuster(0);
  adjuster->moveToThread(this);
  // adjuster cannot be owned by us, because it must be moved into the thread
  pDebug() << "InterruptableAdjuster" << QThread::currentThread() << this;
  start();
}

InterruptableAdjuster::~InterruptableAdjuster() {
  stop();
  if (!wait(1000)) {
    COMPLAIN("InterruptableAdjuster: Failed to stop thread; terminating.");
    terminate();
    if (!wait(1000))
      CRASH("InterruptableAdjuster: Still failed to stop thread");
  }
  delete adjuster;
}

void InterruptableAdjuster::requestFull(AllAdjustments const &settings,
                                        quint64 id) {
  requestReducedROI(settings, QRect(), PSize(), id);
}

void InterruptableAdjuster::requestReduced(AllAdjustments const &settings,
                                           PSize maxSize, quint64 id) {
  requestReducedROI(settings, QRect(), maxSize, id);
}
  
void InterruptableAdjuster::requestROI(AllAdjustments const &settings,
				       QRect roi, quint64 id) {
  requestReducedROI(settings, roi, PSize(), id);
}

void InterruptableAdjuster::requestReducedROI(AllAdjustments const &settings,
                                              QRect roi, PSize maxSize,
                                              quint64 id) {
  QMutexLocker l(&mutex);
  adjuster->cancel();
  newreq = true;
  cancel = false;
  rqAdjustments = settings;
  rqRect = roi;
  rqSize = maxSize;
  rqId = id;
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
  waitcond.wakeOne();
}

PSize InterruptableAdjuster::maxAvailableSize(Adjustments const &s,
                                              quint64 id) {
   if (clear_ || id!=oId)
     return PSize();
   else
     return Geometry::scaledCroppedSize(oSize, s, scaledOSize);
}

bool InterruptableAdjuster::isEmpty() {
  QMutexLocker l(&mutex);
  return empty;
}

void InterruptableAdjuster::setOriginal(Image16 img, quint64 id) {
  setReduced(img, PSize(), id);
}

void InterruptableAdjuster::setReduced(Image16 img, PSize siz, quint64 id) {
  QMutexLocker l(&mutex);
  newOriginal = img;
  scaledOSize = img.size();
  oSize = siz;
  oId = id;
}    

void InterruptableAdjuster::handleNewRequest() {
  QRect r = rqRect;
  PSize s = rqSize;
  bool cando = rqId==oId;

  AllAdjustments sli = rqAdjustments;
  
  newreq = false;

  mutex.unlock();
  Image16 img;
  if (cando) {
    if (r.isEmpty()) {
      if (s.isEmpty())
	img = hnrFull(sli);
      else
	img = hnrReduced(sli, s);
    } else {
      pDebug() << "IA: ROIs NYI";
      /*
      if (s.isEmpty())
        img = adjuster->retrieveROI(sli, r);
      else
        img = adjuster->retrieveReducedROI(sli, r, s);
      */
    }
  } else {
    pDebug() << "InterruptableAdjuster: no can do";
  }
  mutex.lock();

  if (!cando || cancel || newreq || clear_) {
    cancel = false;
    if (clear_) {
      adjuster->clear();
      clear_ = false;
    }
  } else {
    quint64 id = oId;
    QSize fs = Geometry::croppedSize(oSize, sli.baseAdjustments());

    mutex.unlock();
    emit ready(img, id, fs);
    mutex.lock();
  }
}

Image16 InterruptableAdjuster::hnrFull(AllAdjustments const &sli) {
  return adjuster->retrieveFull(sli);
}

Image16 InterruptableAdjuster::hnrReduced(AllAdjustments const &sli,
					  PSize s) {
  return adjuster->retrieveReduced(sli, s);
}


void InterruptableAdjuster::handleNewImage() {
  if (oSize.isEmpty())
    adjuster->setOriginal(newOriginal);
  else
    adjuster->setReduced(newOriginal, oSize);
  newOriginal = Image16();
}

void InterruptableAdjuster::run() {
  pDebug() << "InterruptableAdjuster::run" << QThread::currentThread() << this;
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
      handleNewImage();
    } else if (newreq) {
      handleNewRequest();
    } else {
      waitcond.wait(&mutex);
    }
  }
  mutex.unlock();
}


