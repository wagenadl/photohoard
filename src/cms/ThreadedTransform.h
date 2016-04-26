// ThreadedTransform.h

#ifndef THREADEDTRANSFORM_H

#define THREADEDTRANSFORM_H

#include <QThread>
#include "CMSTransform.h"
#include "Image16.h"
#include <QMutex>
#include <QWaitCondition>

class ThreadedTransform: public QThread {
  Q_OBJECT;
public:
  ThreadedTransform(QObject *parent);
  virtual ~ThreadedTransform();
  quint64 request(Image16 img);
  void cancel(quint64 id);
signals:
  void available(quint64 id, Image16 img);
private:
  virtual void run() override;
private:
  QMutex mutex;
  QWaitCondition waiter;
  Image16 rqimg;
  quint64 rqid;
  Image16 workimg;
  quint64 workid; // nonzero if working
  quint64 nextid;
  bool stopsoon;
  bool cancelflag;
};

#endif
