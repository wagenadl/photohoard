// InterruptableReader.cpp

#include "InterruptableReader.h"
#include <QMutexLocker>
#include <QDebug>

InterruptableReader::InterruptableReader(QObject *parent):
  QThread(parent) {
  state_ = State::Waiting;
  requested = "";
  cancelsoon = false;
  start();
}

InterruptableReader::~InterruptableReader() {
  stop();
  if (!wait(1000)) {
    qDebug() << "InterruptableReader: Failed to stop thread; terminating.";
    terminate();
    if (!wait(1000))
      qDebug() << "InterruptableReader: Still failed to stop thread";
  }
}

QByteArray InterruptableReader::readAll(QString fn) {
  qDebug() << "IR::readAll" << fn;
  QMutexLocker l(&mutex);
  if (state_==State::Success && current==fn) {
    QByteArray result = dest;
    state_ = State::Waiting;
    current = "";
    return result;
  } else {
    return QByteArray();
  }
}

QString InterruptableReader::errorMessage(QString fn) const {
  QMutexLocker l(&mutex);
  if (state_==State::Failed && current==fn)
    return errmsg;
  else
    return QString();
}

void InterruptableReader::request(QString fn) {
  qDebug() << "IR::request" << fn;
  QMutexLocker l(&mutex);
  requested = fn;
  if (state_ != State::Running) 
    waitcond.wakeOne();
}

void InterruptableReader::cancel() {
  qDebug() << "IR::cancel";
  QMutexLocker l(&mutex);
  cancelsoon = true;
  if (state_ != State::Running) 
    waitcond.wakeOne();
}

void InterruptableReader::cancel(QString fn) {
  qDebug() << "IR::cancel" << fn;
  QMutexLocker l(&mutex);
  if (requested==fn)
    requested="";
  if (current==fn)
    cancelsoon = true;
  if (state_ != State::Running) 
    waitcond.wakeOne();
}

void InterruptableReader::start() {
  stopsoon = false;
  QThread::start();
}

void InterruptableReader::stop() {
  stopsoon = true;
  waitcond.wakeOne();
}

//////////////////////////////////////////////////////////////////////
// Thread code
void InterruptableReader::cancelCurrent() {
  cancelsoon = false;
  if (state_==State::Running) {
    mutex.unlock();
    stopSource();
    source().close();
    mutex.lock();
  }
  if (state_!=State::Waiting) {
    dest.clear();
    state_ = State::Waiting;
  }
}

void InterruptableReader::readSome() {
  qint64 N = nextChunkSize();
  if (N+offset > reservedsize) {
    reservedsize *= 2;
    dest.resize(reservedsize);
  }

  mutex.unlock();
  qint64 n = source().read(dest.data()+offset, N);
  mutex.lock();

  if (n<0) { // error
    bool cnc = cancelsoon;
    cancelsoon = false;
    if (cnc) {
      state_ = State::Canceled;
    } else {
      state_ = State::Failed;
      errmsg = "Read error";
    }
    mutex.unlock();
    stopSource();
    source().close();
    dest.clear();
    if (cnc)
      emit canceled(current);
    else
      emit failed(current);
    mutex.lock();
  } else {
    offset += n;
    if (atEnd())
      complete();
  }
}

void InterruptableReader::complete() {
  bool cnc = cancelsoon;
  cancelsoon = false;
  if (cnc)
    state_ = State::Canceled;
  else
    state_ = State::Success;

  mutex.unlock();
  source().close();
  if (cnc) {
    dest.clear();
    emit canceled(current);
  } else {
    dest.resize(offset);
    emit ready(current);
  }
  mutex.lock();
}


void InterruptableReader::newRequest() {
  cancelCurrent();
  
  state_ = State::Running;
  current = requested;
  requested = "";

  mutex.unlock();
  offset = 0;
  if (!openCurrent()) {
    mutex.lock();
    state_ = State::Failed;
    errmsg = "Could not open file";
    mutex.unlock();
    emit failed(current);
  }
  reservedsize = dest.size();
  mutex.lock();
}

void InterruptableReader::run() {
  mutex.lock();
  while (!stopsoon) {
    if (cancelsoon) 
      cancelCurrent();
    else if (requested!="")
      newRequest();
    else if (state_==State::Running)
      readSome();
    else
      waitcond.wait(&mutex);
  }
  mutex.unlock();
}
