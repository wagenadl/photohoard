// InterruptableReader.h

#ifndef INTERRUPTABLEREADER_H

#define INTERRUPTABLEREADER_H

#include <QObject>
#include <QIODevice>

class InterruptableReader: public QObject {
  Q_OBJECT;
public:
  enum class State {
    Waiting,
    Running,
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
public slots:
  void request(QString fn);
  // Posting a new request cancels the previous.
  void cancel(QString fn);
  // Has no effect if FN is not the current request.
  void cancel();
  // Cancels no matter what
signals:
  void ready(QString fn, QByteArray data);
  // Emitted when state transitions to Success.
  void failed(QString fn, QString message);
  // Emitted when state transitions to Failed.
  void canceled(QString fn);
  // Emitted when request() or cancel() cancels a Running/Success/Failed state.
  void readMore(); // private
private slots:
  void readSome();
protected:
  virtual QIODevice &source()=0; // valid _only_ in state Running
  virtual void abort() { }
  virtual bool open()=0;
private:
  void complete();
protected:
  QString requested;
  State state_;
  QByteArray dest;
  qint64 reservedsize, offset;
};

#endif
