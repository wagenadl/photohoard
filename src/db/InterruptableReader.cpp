// InterruptableReader.cpp

#include "InterruptableReader.h"
#include <QMutexLocker>
#include <QDebug>
#include <unistd.h>

InterruptableReader::InterruptableReader(QObject *parent):
  QThread(parent) {
  running = false;
  canceling = false;
  stopsoon = false;
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
  QMutexLocker l(&mutex);
  stopsoon = true;
  cond.wakeOne();
}

void InterruptableReader::request(QString fn, QSize request, QSize original) {
  QMutexLocker l(&mutex);
  newreq = fn;
  rqSize = request;
  oriSize = original;
  if (running) {
    abort();
  } else {
    // Not running, so we may access the data since we have the mutex
    if (res.ok) 
      res = Result("Superseded");
    cond.wakeOne();
  }
}

void InterruptableReader::cancel() {
  QMutexLocker l(&mutex);
  newreq = "";
  if (running) {
    abort();
    canceling = true;
  } else {
    // Not running, so we may access the data since we have the mutex
    if (res.ok) 
      res = Result("Canceled");
  }
}


void InterruptableReader::cancel(QString fn) {
  QMutexLocker l(&mutex);
  if (fn==newreq) {
    newreq = "";
  } else if (fn==current) {
    if (running) {
      abort();
      canceling = true;
    } else {
      // Not running, so we may access the data since we have the mutex
      if (res.ok) 
	res = Result("Canceled");
    }
  }
}

InterruptableReader::Result InterruptableReader::result(QString fn) {
  QMutexLocker l(&mutex);
  if (running)
    return Result("Incomplete");
  if (fn!=current)
    return Result("Not current");
  Result r = res;
  res = Result();
  return r;
}

//////////////////////////////////////////////////////////////////////
// Thread code

void InterruptableReader::run() {
  mutex.lock();
  while (!stopsoon) {
    if (newreq != "") {
      lNewReq();
    } else if (canceling) {
      lCancel();
    } else if (running) {
      lReadSome();
    } else {
      cond.wait(&mutex);
    }
  }
  mutex.unlock();
}

void InterruptableReader::lNewReq() {
  qDebug() << "IR " << " lNewReq" << newreq;
  if (running) {
    abort();
    running = false;
    if (newreq=="") { // this can happen if the request is canceled
      // while we were processing the abort.
      lCancel();
      return;
    }
  }

  res = Result("Incomplete");
  offset = 0;
  current = newreq;
  newreq = "";
  canceling = false;
  running = true;
  lPrepSource(current, rqSize, oriSize);
  
  mutex.unlock();

  if (uOpen()) {
    estsize = uEstimateSize();
    if (estsize)
      res.data.resize(estsize);
    else
      res.data.resize(8*1024*1024);
    mutex.lock();
  } else {
    mutex.lock();
    res = Result("Could not open");
    running = false;
    lUnprepSource();
    QString c = current;
    mutex.unlock();
    emit failed(c);
    mutex.lock();
  }
}

void InterruptableReader::lCancel() {
  qDebug() << "IR " << " lCancel" << current;
  if (running) 
    abort();

  current = "";
  res = Result("Canceled");
  canceling = false;
  running = false;
  lUnprepSource();
}

void InterruptableReader::lReadSome() {
  mutex.unlock();
  qint64 chunksize = 512*1024;
  if (estsize>0 && offset+chunksize>estsize)
    chunksize = estsize - offset;
  if (chunksize>0 && !tSource().atEnd()) {
    while (offset+chunksize>res.data.size()) 
      res.data.resize(2*res.data.size());
    int n = tSource().read(res.data.data()+offset, chunksize);

    mutex.lock();

    if (n<0) {
      qDebug() << "IR: Read error";
      // Error
      res = Result("Read error");
      lUnprepSource();
      running = false;
      QString c = current;
      mutex.unlock();
      emit failed(c);
      mutex.lock();
    } else if (n==0) {
      qDebug() << "IR: Read nothing";
      if (tSource().atEnd())
	lComplete();
    } else {
      offset += n;
    }
  } else {
    mutex.lock();
    lComplete();
  }
}

void InterruptableReader::lComplete() {
  qDebug() << "IR " << " lComplete" << current << offset << estsize;
  mutex.unlock();
  tSource().close();
  mutex.lock();

  if (canceling) {
    lCancel();
  } else {
    res.ok = true;
    res.error = "";
    lUnprepSource();
    running = false;
    QString c = current;
    mutex.unlock();
    emit ready(c);
    mutex.lock();
  }
}
