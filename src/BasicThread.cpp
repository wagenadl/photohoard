// BasicThread.cpp

#include "BasicThread.h"

#include <QDebug>

BasicThread::BasicThread(QObject *parent): QThread(parent) {
}

BasicThread::~BasicThread() {
  if (isRunning())
    stop();
  if (!wait(1000)) 
    qDebug() << "Failed to stop thread " << objectName();
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
