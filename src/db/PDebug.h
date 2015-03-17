// PDebug.h

#ifndef PDEBUG_H

#define PDEBUG_H

#include <QDebug>

QDebug pDebug();

namespace PDebug {
  void reset();
  QTime &time();
}

#endif
