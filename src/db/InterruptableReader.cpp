// InterruptableReader.cpp

#include "InterruptableReader.h"
#include <QMutexLocker>
#include <QDebug>
#include <unistd.h>

InterruptableReader::InterruptableReader(QObject *parent):
  QObject(parent) {
  state_ = State::Waiting;
  requested = "";
  connect(this, SIGNAL(readMore()), SLOT(readSome()), Qt::QueuedConnection);
}

InterruptableReader::~InterruptableReader() {
}

void InterruptableReader::request(QString fn) {
  if (state_==State::Running)
    cancel();

  state_ = State::Running;
  requested = fn;
  offset = 0;
  if (!open()) {
    requested = "";
    state_ = State::Waiting;
    emit failed(fn, "Could not open source");
  }
  qDebug() << "requested ok";
  reservedsize = dest.size();
  readSome();
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
