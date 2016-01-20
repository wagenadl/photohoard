// BasicThread.h

#ifndef BASICTHREAD_H

#define BASICTHREAD_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

class BasicThread: public QThread {
  Q_OBJECT;
public:
  BasicThread(QObject *parent=0);
  virtual ~BasicThread();
  bool stopAndWait(int timeout_ms=1000);
public slots:
  void start();
  void stop();
protected:
  bool stopsoon;
  QMutex mutex;
  QWaitCondition waiter;
};

#endif
