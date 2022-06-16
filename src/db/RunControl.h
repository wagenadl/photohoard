// Runcontrol.h

#ifndef RUNCONTROL_H

#define RUNCONTROL_H

#include <Qt>

class RunControl {
public:
  static bool isRunning(quint64 pid);
  static quint64 pid();
};
#endif
