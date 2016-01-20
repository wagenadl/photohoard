// NiceProcess.cpp

#include "NiceProcess.h"
#include <unistd.h>

NiceProcess::NiceProcess() {
  nice = 0;
}

void NiceProcess::renice(int n) {
  nice = n;
}

void NiceProcess::setupChildProcess() {
  if (nice) {
    if (::nice(nice)) {
      ;
    }
  }
}
