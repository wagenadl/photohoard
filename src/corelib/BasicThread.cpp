// BasicThread.cpp

#include "BasicThread.h"

#include "PDebug.h"

BasicThread::BasicThread(QObject *parent): QThread(parent) {
}

BasicThread::~BasicThread() {
  stopAndWait(1000);
}

bool BasicThread::stopAndWait(int timeout_ms) {
  if (!isRunning())
    return true;
  stop();
  if (wait(timeout_ms))
    return true;
  COMPLAIN("Failed to stop thread " + objectName());
  return false;
}

void BasicThread::start() {
  if (!isRunning()) {
    stopsoon = false;
    QThread::start();
  }
}

void BasicThread::stop() {
  if (isRunning()) {
    QMutexLocker l(&mutex);
    stopsoon = true;
    waiter.wakeOne();
  }
}
