// InterruptableReader.cpp

#include "InterruptableReader.h"
#include <QMutexLocker>
#include <QDebug>
#include <unistd.h>

InterruptableReader::InterruptableReader(QObject *parent):
  QObject(parent) {
  running = false;
  canceling = false;
  stopsoon = false;
  result.ok = false;
  start();
}

InterruptableReader::~InterruptableReader() {
  stop();
  if (!wait(1000)) {
    qDebug() << "InterruptableReader deleted; did not stop; terminating.";
    terminate();
    if (!wait(1000)) 
      qDebug() << "InterruptableReader still did not stop; disaster imminent.";
  }
}

void InterruptableReader::start() {
  QThread::start();
}

void InterruptableReader::stop() {
  stopsoon = true;
  cond.wakeOne();
}

void InterruptableReader::request(QString fn) {
  QStringList canc;
  QMutexLocker l(&mutex);
  if (fn==current) {
    canc << current;
    if (running)
      canceling = true;
  }
  if (fn==newreq) {
    canc << newreq;
    newreq = "";
  }

  newreq = fn;
  wakeOne();
  mutex.unlock();
  for (auto s: canc)
    emit canceled(s);
}

void InterruptableReader::cancel() {
  abort();
  source().close();
  dest.clear();
  QString fn = requested;
  requested = "";
  state_ = State::Waiting;
  emit canceled(fn);
}

void InterruptableReader::cancel(QString fn) {
  if (fn==requested && state_==State::Running)
    cancel();
}

void InterruptableReader::readSome() {
  qint64 N0 = source().bytesAvailable();
  constexpr qint64 Nmax = 1024*1024;
  qint64 N = N0;
  if (N>Nmax)
    N = Nmax;

  if (N+offset > reservedsize) {
    reservedsize *= 2;
    dest.resize(reservedsize);
  }

  if (N>0) {
    qint64 n = source().read(dest.data()+offset, N);
    qDebug() << "IR " << requested << " readsome" << n << N;
    if (n<0) { // error
      abort();
      source().close();
      dest.clear();
      QString fn = requested;
      requested = "";
      state_ = State::Waiting;
      emit failed(fn, "Read error");
    } else {
      offset += n;
    }
  }

  if (source().atEnd())
    complete();
  else if (N0>N)
    emit readMore();
}

void InterruptableReader::complete() {
  source().close();
  QString fn = requested;
  requested = "";
  QByteArray ar = dest;
  dest.clear();
  state_ = State::Waiting;
  emit ready(fn, ar);
}
