// InterruptableReader.h

#ifndef INTERRUPTABLEREADER_H

#define INTERRUPTABLEREADER_H

#include <QThread>
#include <QIODevice>

class InterruptableReader: public QThread {
  Q_OBJECT;
public:
  InterruptableReader(QObject *parent=0);
  // Starts thread.
  virtual ~InterruptableReader();
  // Stops thread canceling outstanding requests.
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
protected:
  virtual QIODevice &source()=0; // valid _only_ in state Running
  virtual void abort() { }
  virtual bool open()=0;
private:
  void complete();
protected:
  QString newreq;
  QString current;
  bool running, cancelsoon;
  QByteArray dest;
  qint64 reservedsize, offset;
};

#endif
