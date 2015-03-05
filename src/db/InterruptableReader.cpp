// InterruptableReader.cpp

#include "InterruptableReader.h"
#include <QMutexLocker>
#include <QDebug>

InterruptableReader::InterruptableReader(QObject *parent):
  QThread(parent) {
  state_ = State::Waiting;
  requested = "";
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
  QMutexLocker l(&mutex);
  if (state_==State::Success && current==fn) {
    QByteArray result = dest;
    state_ = State::Waiting;
    requested = "";
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
  QMutexLocker l(&mutex);
  requested = fn;
  if (state_ != State::Waiting && state_ != State::Canceled) {
    stopSource();
    state_ = State::Canceled;
    QString cur = current;
    l.unlock();
    emit canceled(cur);
    l.relock();
  }
  if (state_ != State::Running) 
    waitcond.wakeOne();
}

void InterruptableReader::cancel() {
  mutex.lock();
  QString c = current;
  mutex.unlock();
  cancel(current);
}

void InterruptableReader::cancel(QString fn) {
  QMutexLocker l(&mutex);

  if (requested==fn)
    requested="";
  if (state_==State::Waiting || state_==State::Canceled)
    return;
  if (current!=fn)
    return;

  stopSource();
  state_ = State::Canceled;
  QString cur = current;
  l.unlock();
  emit canceled(cur);
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
  if (state_==State::Running || state_==State::Canceled) {
    mutex.unlock();
    source().close();
    dest.clear();
    mutex.lock();
  }
  state_ = State::Waiting;
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
    bool cnc = state_==State::Canceled;
    if (cnc) {
      state_ = State::Waiting;
    } else {
      state_ = State::Failed;
      errmsg = "Read error";
    }
    mutex.unlock();
    source().close();
    dest.clear();
    if (!cnc)
      emit failed(current);
    mutex.lock();
  } else {
    offset += n;
    if (source().atEnd())
      complete();
  }
}

void InterruptableReader::complete() {
  bool cnc = state_==State::Canceled;
  if (!cnc)
    state_ = State::Success;

  mutex.unlock();
  source().close();
  if (cnc) {
    dest.clear();
  } else {
    dest.resize(offset);
    emit ready(current);
  }
  mutex.lock();
}


void InterruptableReader::newRequest() {
  state_ = State::Running;
  current = requested;

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
    if (state_!=State::Waiting && requested!=current) 
      cancelCurrent();
    
    if (state_==State::Running) {
      readSome();
    } else {
      if (requested=="") 
        waitcond.wait(&mutex);
      else
        newRequest();
    }
  }
  mutex.unlock();
}
