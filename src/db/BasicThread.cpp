// BasicThread.cpp

#include "BasicThread.h"

#include <QDebug>

BasicThread::BasicThread(QObject *parent): QThread(parent) {
}

BasicThread::~BasicThread() {
  stopAndWait(1000);
}

bool BasicThread::stopAndWait(int timeout_ms) {
  qDebug() << "stopandwait";
  if (!isRunning())
    return true;
  qDebug() << "  running";
  stop();
  qDebug() << "  sent stop request";
  if (wait(timeout_ms))
    return true;
  qDebug() << "Failed to stop thread " << objectName();
    return false;
}

void BasicThread::start() {
  if (!isRunning()) {
    stopsoon = false;
    QThread::start();
  }
}

void BasicThread::stop() {
  qDebug() << "BT::stop";
  if (isRunning()) {
    qDebug() << "  running";
    QMutexLocker l(&mutex);
    qDebug() << "  got mutex";
    stopsoon = true;
    waiter.wakeOne();
    qDebug() << "  sent wakeup";
  }
}
