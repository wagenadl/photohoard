// BasicThread.cpp

#include "BasicThread.h"

#include "PDebug.h"

BasicThread::BasicThread(QObject *parent): QThread(parent) {
}

BasicThread::~BasicThread() {
  stopAndWait(1000);
}

bool BasicThread::stopAndWait(int timeout_ms) {
  pDebug() << "stopandwait";
  if (!isRunning())
    return true;
  pDebug() << "  running";
  stop();
  pDebug() << "  sent stop request";
  if (wait(timeout_ms))
    return true;
  pDebug() << "Failed to stop thread " << objectName();
    return false;
}

void BasicThread::start() {
  if (!isRunning()) {
    stopsoon = false;
    QThread::start();
  }
}

void BasicThread::stop() {
  pDebug() << "BT::stop";
  if (isRunning()) {
    pDebug() << "  running";
    QMutexLocker l(&mutex);
    pDebug() << "  got mutex";
    stopsoon = true;
    waiter.wakeOne();
    pDebug() << "  sent wakeup";
  }
}
