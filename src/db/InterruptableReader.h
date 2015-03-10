// InterruptableReader.h

#ifndef INTERRUPTABLEREADER_H

#define INTERRUPTABLEREADER_H

#include <QThread>
#include <QIODevice>
#include <QMutex>
#include <QWaitCondition>

class InterruptableReader: public QThread {
  Q_OBJECT;
public:
  class Result {
  public:
    QByteArray data;
    bool ok;
    QString error;
  };
public:
  InterruptableReader(QObject *parent=0);
  // Starts thread.
  virtual ~InterruptableReader();
  // Stops thread canceling outstanding requests.
  Result result() const;
public slots:
  void request(QString fn);
  // Posting a new request cancels the previous.
  void cancel(QString fn);
  // Has no effect if FN is not the current request.
  void cancel();
  // Cancels no matter what
signals:
  void ready(QString fn);
  void failed(QString fn);
  void canceled(QString fn);
  // Each request results in precisely one such signal
protected:
  virtual QIODevice &source()=0;
  virtual void abort() { }
  virtual bool open()=0;
private:
  void tComplete();
  void start();
  void stop();
  virtual void run();
protected:
  QString newreq;
  QString current;
  bool running, canceling, stopsoon;
  Result result;
  qint64 reservedsize, offset;
  QMutex mutex;
  QWaitCondition cond;
};

#endif
