// InterruptableReader.h

#ifndef INTERRUPTABLEREADER_H

#define INTERRUPTABLEREADER_H

#include <QThread>
#include <QFile>
#include <QWaitCondition>
#include <QMutex>

class InterruptableReader: public QThread {
  Q_OBJECT;
public:
  enum class State {
    Waiting,
    Running,
    Success,
    Canceled,
    Failed,
  };    
public:
  InterruptableReader(QObject *parent=0);
  // Starts thread.
  virtual ~InterruptableReader();
  // Stops thread canceling outstanding requests.
  State state() const { return state_; }
  /* Note that state() returns the current state; because of multithreading,
     this is not a guarantee for future results of readAll().
     However, our child thread will not change state Success to anything
     else except when provoked by the caller through request(), cancel(), or
     readAll().
  */
  QByteArray readAll(QString fn);
  // Only successful in Success state.
  // Resets state to Waiting.
  QString errorMessage(QString fn) const;
  // Only successful in Failed state.
public slots:
  void request(QString fn);
  // Posting a new request cancels the previous.
  void cancel(QString fn);
  // Has no effect if FN is not the current request.
  void cancel();
  // Cancels no matter what
signals:
  void ready(QString fn);
  // Emitted when state transitions to Success.
  void failed(QString fn);
  // Emitted when state transitions to Failed.
  void canceled(QString fn);
  // Emitted when request() or cancel() cancels a Running/Success/Failed state.
private:
  virtual void start();
  virtual void stop();
  virtual void run();
protected:
  virtual QIODevice &source()=0; // valid _only_ in state Running
  virtual void stopSource() { }
  virtual qint64 nextChunkSize() { return 65536; }
  virtual bool openCurrent()=0;
  virtual bool atEnd() const=0;
private:
  void cancelCurrent();
  void readSome();
  void complete();
  void newRequest();
protected:
  QString requested; // always valid
  QString current; // valid _except_ in state Waiting
  State state_;
  QByteArray dest; // valid _only_ in state Running and Success
  mutable QMutex mutex;
  QWaitCondition waitcond;
  QString errmsg; // valid _only_ in state Failed
  bool stopsoon;
  bool cancelsoon;
  qint64 reservedsize, offset;
};

#endif
