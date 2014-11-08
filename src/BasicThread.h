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
public slots:
  void start();
  void stop();
protected:
  bool stopsoon;
  QMutex mutex;
  QWaitCondition waiter;
};

#endif
