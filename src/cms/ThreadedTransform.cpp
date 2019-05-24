// ThreadedTransform.cpp

#include "ThreadedTransform.h"
#include <QMutexLocker>
#include <vector>
#include <future>
#include <algorithm>
#include "PDebug.h"
#include "CMSTransform.h"
#include "CMS.h"

ThreadedTransform::ThreadedTransform(QObject *parent): QThread(parent) {
  rqid = 0;
  workid = 0;
  nextid = 1;
  stopsoon = false;
  cancelflag = false;
}

ThreadedTransform::~ThreadedTransform() {
  if (isRunning()) {
    stopsoon = true;
    cancel(0);
    COMPLAIN("ThreadedTransform: Destructed while running. Waiting.");
    wait(5000);
  }
}

quint64 ThreadedTransform::request(Image16 img) {
  //pDebug() << "TT::rq";
  QMutexLocker lck(&mutex);
  //  pDebug() << "TT::rq: got lock";
  rqid = ++nextid;
  rqimg = img;
  cancelflag = true;
  if (isRunning())
    waiter.wakeOne();
  else
    start();
  //  pDebug() << "TT::returning" << rqid;
  return rqid;
}

void ThreadedTransform::cancel(quint64 id) {
  QMutexLocker lck(&mutex);
  if (workid==id || id==0) {
    rqid = 0;
    cancelflag = true;
    rqimg = Image16();
    waiter.wakeOne();
  }
}

static int runabit(uchar *bit, int npix, bool *cancelflag) {
  //  pDebug() << "TT:runabit" << bit << npix;
  while (npix>0) {
    if (*cancelflag)
      return 0;
    int now = std::min(npix, 64*1024);
    CMS::monitorTransform.apply(bit, bit, now);
    bit += 4*now;
    npix -= now;
  }
  return 1;
}

void ThreadedTransform::run() {
  mutex.lock();
  while (!stopsoon) {
    //    pDebug() << "TT:rqid=" << rqid;
    if (rqid) {
      cancelflag = false;
      workid = rqid;
      workimg = rqimg.convertedTo(Image16::Format::sRGB8);
      rqimg = Image16();
      mutex.unlock();
      //      pDebug() << "TT:constructed workimg" << workimg.size();
      
      uchar *ptr = workimg.bytes();
      int npix = workimg.bytesPerLine() * workimg.height() / 4;
      std::vector< std::future<int> > futures;
      constexpr int NTHREADS = 4;
      int pixperrun = std::max((npix+NTHREADS-1)/NTHREADS, 512*1024);
      while (npix>0) {
        int now = std::min(pixperrun, npix);
	//        pDebug() << "TT:adding" << now << npix;
        futures.push_back(std::async(std::launch::async, runabit,
                                     ptr, now, &cancelflag));
        ptr += 4*now;
        npix -= now;
      }
      
      bool ok = true;
      //      pDebug() << "TT: getting";
      for (auto &f: futures) {
        if (!f.get())
          ok = false;
	//        pDebug() << "TT: got" << ok;
      }
      //      pDebug() << "TT: got all" << ok;
      mutex.lock();

      if (ok && rqid==workid) {
        quint64 doneid = workid;
        workid = 0;
        rqid = 0;
        rqimg = Image16();
        mutex.unlock();
	//        pDebug() << "TT: emitting";
        emit available(doneid, workimg);
        mutex.lock();
      } 
    }
    if (rqid==0 && !stopsoon) {
      //      pDebug() << "TT:waiting";
      waiter.wait(&mutex);
    }
  }
  mutex.unlock();
}
