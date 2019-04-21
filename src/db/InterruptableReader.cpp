// InterruptableReader.cpp

#include "InterruptableReader.h"
#include <QMutexLocker>
#include "PDebug.h"
#include <unistd.h>

InterruptableReader::InterruptableReader(QObject *parent):
  QThread(parent) {
  qRegisterMetaType<InterruptableReader::Result>("InterruptableReader::Result");
  running = false;
  canceling = false;
  stopsoon = false;
  start();
}

InterruptableReader::~InterruptableReader() {
  stop();
  if (!wait(1000)) {
    COMPLAIN("InterruptableReader deleted; did not stop; terminating.");
    terminate();
    if (!wait(1000)) 
      CRASH("InterruptableReader still did not stop; disaster.");
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
  // pDebug() << "IR::request" << fn << request << original << running; 
  QMutexLocker l(&mutex);  
  if (current==fn && cSize==request) {
    // pDebug() << "Same request again" << canceling;
    // same as previous, that's easy
    if (running) {
      // pDebug() << "IR::req returning";
      // Is it OK that the signal will be emitted only once? I hope so.
      return;
    } else if (res.ok) {
      // pDebug() << "IR::req emitting";
      Result r = res;
      l.unlock();
      emit ready(fn, r);
      return;
    } else {
      // pDebug() << "IR::req res not ok";
    }
  }

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
  // pDebug() << "IR::cancel";
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
  // pDebug() << "IR::cancel" << fn;
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
  if (running) {
    abort();
    running = false;
    if (newreq=="") { // this can happen if the request is canceled
      // while we were processing the abort.
      lCancel();
      return;
    }
  }

  // pDebug() << "IR::lNR" << newreq << rqSize;
  res = Result("Incomplete");
  offset = 0;
  current = newreq;
  cSize = rqSize;
  newreq = "";
  canceling = false;
  running = true;
  lPrepSource(current, cSize, oriSize);
  // pDebug() << "IR: source prepped";
  mutex.unlock();

  if (uOpen()) {
    estsize = uEstimateSize();
    if (estsize)
      res.data.resize(estsize);
    else
      res.data.resize(8*1024*1024);
    // pDebug() << "IR: resized with estsize " << estsize;
    mutex.lock();
  } else {
    mutex.lock();
    res = Result("Could not open");
    running = false;
    lUnprepSource();
    QString c = current;
    mutex.unlock();
    COMPLAIN("Interruptable reader failed: " + c);
    emit failed(c);
    mutex.lock();
  }
}

void InterruptableReader::lCancel() {
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
  // pDebug() << " IR::lRS" << offset << chunksize;
  if (chunksize>0 && !tSource().atEnd()) {
    while (offset+chunksize>res.data.size()) 
      res.data.resize(2*res.data.size());
    int n = tSource().read(res.data.data()+offset, chunksize);

    mutex.lock();

    if (n<0) {
      COMPLAIN("IR: Read error");
      // Error
      res = Result("Read error");
      lUnprepSource();
      running = false;
      QString c = current;
      mutex.unlock();
      COMPLAIN("Interruptable reader failed1: " + c);
      emit failed(c);
      mutex.lock();
    } else if (n==0) {
      COMPLAIN("IR: Read nothing");
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
  //  pDebug() << "IR::lC";
  mutex.unlock();
  tSource().close();
  mutex.lock();
  if (canceling) {
    //    pDebug() << "IR::LC->cancel";
    lCancel();
    return;
  }

  mutex.unlock();
  //  pDebug() << "IR::lC will load";
  res.image = Image16::loadFromMemory(res.data);
  //  pDebug() << "IR::LC loaded";
  res.data.clear();
  mutex.lock();

  if (canceling) {
    //    pDebug() << "IR::LC->cancel 2";
    lCancel();
    return;
  }

  res.ok = true;
  res.error = "";
  lUnprepSource();
  //  pDebug() << "IR::lC ->unprep";
  running = false;

  QString c = current;
  Result r = res;
  mutex.unlock();
  //  pDebug() << "Interruptable reader ready" << c;
  emit ready(c, r);
  //  pDebug() << "Back from ready";
  mutex.lock();
  //  pDebug() << "IR: Got mutex again";
}
