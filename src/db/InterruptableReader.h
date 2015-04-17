// InterruptableReader.h

#ifndef INTERRUPTABLEREADER_H

#define INTERRUPTABLEREADER_H

#include <QThread>
#include <QIODevice>
#include <QMutex>
#include <QWaitCondition>
#include <QSize>
#include "Image16.h"

class InterruptableReader: public QThread {
  Q_OBJECT;
public:
  class Result {
  public:
    Result(QString e="Not started") { ok = false; error = e; }
  public:
    QByteArray data; // set to null after image constructed
    Image16 image; // constructed just before emitting "ready"
    bool ok;
    QString error;
  };
public:
  InterruptableReader(QObject *parent=0);
  // Starts thread.
  virtual ~InterruptableReader();
  // Stops thread canceling outstanding requests.
  Result result(QString fn); // Results may only be fetched once!
public slots:
  void request(QString fn, QSize desired=QSize(), QSize original=QSize());
  // Posts a new request canceling any current or pending requests.
  // Note that the reader doesn't know about exif orientation.
  void cancel(QString fn);
  // Has no effect if FN is not the current or pending request.
  void cancel(); // Cancels everything
signals:
  void ready(QString fn);
  void failed(QString fn);
  // Each request results in either a ready or a failed signal, unless it
  // gets canceled first. The signals are emitted while the mutex is unlocked.
protected:
  virtual void abort() { }
  /* Abort is unique in that it can be used by caller or thread. It is
     supposed to kill the process that produces the data that we will
     read. It must _not_ close the source.
  */
  /* Methods starting with "l" are called by the thread with the mutex locked;
     those with "u" are called by the thread with the mutex unlocked; those
     with "t" are called with the mutex locked or unlocked.
   */
  virtual QIODevice &tSource()=0;
  // called with the mutex locked or unlocked, only after source is prepped  
  virtual bool uOpen()=0;
  virtual void lPrepSource(QString fn, QSize desired, QSize original)=0; 
  virtual void lUnprepSource() { }
  virtual int uEstimateSize() { return 0; }
  // called with the mutex unlocked, only after uOpen is successful.
private:
  void start();
  void stop();
  virtual void run();
private:
  void lCancel();
  void lNewReq(); 
  void lReadSome();
  void lComplete();
private:
  QString newreq;
  QSize rqSize, oriSize;
  QString current;
  bool running, canceling, stopsoon;
  Result res;
  qint64 offset, estsize;
  QMutex mutex;
  QWaitCondition cond;
  /* Rules of the road:
     When running=true, only the thread may access res.data. Otherwise,
     res.data may only be accessed while holding the mutex.
     The mutex must be held to access newreq, running, canceling, stopsoon,
     res.ok and res.error, and current.
     Only the thread may access offset, estsize.
     The client may not change anything except canceling, newreq, stopsoon,
     and---if not running---res.
  */
};

#endif
