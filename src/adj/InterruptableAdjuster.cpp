// InterruptableAdjuster.cpp

#include "InterruptableAdjuster.h"
#include "Adjuster.h"
#include <QMutexLocker>
#include "PDebug.h"

InterruptableAdjuster::InterruptableAdjuster(QObject *parent):
  QThread(parent) {
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
}

void InterruptableAdjuster::requestFull(QMap<int, Adjustments> settings,
                                        quint64 id) {
  requestReducedROI(settings, QRect(), PSize(), id);
}

void InterruptableAdjuster::requestReduced(QMap<int, Adjustments> settings,
                                           PSize maxSize, quint64 id) {
  requestReducedROI(settings, QRect(), maxSize, id);
}
  
void InterruptableAdjuster::requestROI(QMap<int, Adjustments> settings,
				       QRect roi, quint64 id) {
  requestReducedROI(settings, roi, PSize(), id);
}

void InterruptableAdjuster::requestReducedROI(QMap<int, Adjustments> settings,
                                              QRect roi, PSize maxSize,
                                              quint64 id) {
  QMutexLocker l(&mutex);
  for (auto adj: adjuster)
    adj->cancel();
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
  for (auto adj: adjuster)
    adj->cancel();
  newreq = false;
  cancel = true;
}

void InterruptableAdjuster::clear() {
  QMutexLocker l(&mutex);
  for (auto adj: adjuster)
    adj->cancel();
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

PSize InterruptableAdjuster::maxAvailableSize(Adjustments const &s,
                                               quint64 id) {
   if (clear_ || id!=oId)
     return PSize();
   else
     return Adjuster::mapCropSize(oSize, s, scaledOSize);
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


void InterruptableAdjuster::adjustLayerCount(QList<int> newlayers) {
  // Make sure we have the right number of layers
  QSet<int> droppable;
  for (auto lay: adjuster.keys())
    droppable.insert(lay);
  for (auto lay: newlayers) {
    if (droppable.contains(lay)) {
      droppable.remove(lay);
    } else {
      adjuster[lay] = new Adjuster;
      adjuster[lay]->setMaxThreads(4);
    }
  }
  for (auto lay: droppable)
    if (lay>0) 
      adjuster[lay]->clear();
  // we won't actually delete them; they'll come in handy again
}

void InterruptableAdjuster::handleNewRequest() {
  QRect r = rqRect;
  PSize s = rqSize;
  bool cando = rqId==oId;

  QMap<int, Adjustments> sli = rqAdjustments;
  ASSERT(sli.contains(0));
  adjustLayerCount(sli.keys());
  
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
      for (auto adj: adjuster)
	adj->clear();
      clear_ = false;
    }
  } else {
    quint64 id = oId;
    QSize fs = Adjuster::mapCropSize(oSize, sli[0]);
    mutex.unlock();
    emit ready(img, id, fs);
    mutex.lock();
  }
}

Image16 InterruptableAdjuster::hnrFull(QMap<int, Adjustments> const &sli) {
  ASSERT(adjuster.contains(0));
  Image16 img = adjuster[0]->retrieveFull(sli[0]);
#if 0
  // WORK IN PROGRESS
  if (sli.size()>1) {
    /* If there are more layers, we must set and retrieve all those
       layers and alphablend with their masks. */
    QList<int> lays = sli.keys();
    lays.deleteFirst(); // drop base layer
    for (int lay: lays) {
      ASSERT(adjuster.contains(lay));
      adjuster[lay]->setImage(img); // in many cases, this is overkill
      // we should have some way to preserve unchanged images
      Image16 ovr = adjuster[lay]->retrieveFull(sli[lay]);
  }
#endif
  return img;
}

Image16 InterruptableAdjuster::hnrReduced(QMap<int, Adjustments> const &sli,
					  PSize s) {
  ASSERT(adjuster.contains(0));
  Image16 img = adjuster[0]->retrieveReduced(sli[0], s);
  return img;
}


void InterruptableAdjuster::handleNewImage() {
  if (!adjuster.contains(0))
    adjustLayerCount(QList<int>() << 0); // ensure we have at least...
  // ...  an adjuster for the base layer
  if (oSize.isEmpty())
    adjuster[0]->setOriginal(newOriginal);
  else
    adjuster[0]->setReduced(newOriginal, oSize);
  newOriginal = Image16();
}

void InterruptableAdjuster::run() {
  mutex.lock();
  while (!stopsoon) {
    empty = adjuster.isEmpty() || adjuster[0]->isEmpty();
    if (clear_) {
      for (auto adj: adjuster)
	adj->clear();
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
  for (auto adj: adjuster)
    delete adj;
  adjuster.clear();
  mutex.unlock();
}


