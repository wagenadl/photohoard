// PDebug.cpp

#include "PDebug.h"
#include <QTime>

namespace PDebug {
  QTime &time() {
    static QTime t0;
    static bool started = false;
    if (!started) {
      t0.start();
      started = true;
    }
    return t0;
  }

  void reset() {
    time().restart();
  }
};

QDebug pDebug() {
  return qDebug() << QString("%1").arg(PDebug::time().elapsed()/1000.0,
                                       7, 'f', 3).toUtf8().data();
}
