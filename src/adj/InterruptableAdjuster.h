// InterruptableAdjuster.h

#ifndef INTERRUPTABLEADJUSTER_H

#define INTERRUPTABLEADJUSTER_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include "Sliders.h"
#include "Image16.h"

class InterruptableAdjuster: public QThread {
  Q_OBJECT;
public:
  InterruptableAdjuster(class Adjuster *adjuster, QObject *parent=0);
  virtual ~InterruptableAdjuster();
  void requestFull(Sliders const &settings);
  void requestReduced(Sliders const &settings, QSize maxSize);
  void requestROI(Sliders const &settings, QRect roi);
  void requestReducedROI(Sliders const &settings, QRect roi, QSize maxSize);
  void cancelRequest();
  void emitWhileLocked();
  void emitWhileUnlocked();
  /* Emitted the ready signal while the mutex is locked guarantees that
     the signal cannot be emitted for an outdated request. However,
     it means that you must NOT post another request from within connected
     slots! (Otherwise dead lock results.)
     On the other hand, emitting the signal while the mutex is unlocked
     means that there is a (slight) possibility of emitting the signal for
     an outdated request, but that there is no risk of dead locks.
     The default is to emit while unlocked.
  */
signals:
  void ready(Image16);
  // Note that "ready" does not imply success: the image can be null.
protected:
  void start();
  void stop();
  virtual void run();
private:
  Adjuster *adjuster;
  QMutex mutex;
  QWaitCondition waitcond;
  bool cancel, newreq;
  Sliders rqSliders;
  QRect rqRect;
  QSize rqSize;
  bool stopsoon;
  bool emit_while_locked;
};

#endif
