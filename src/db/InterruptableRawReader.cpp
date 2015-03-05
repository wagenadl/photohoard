// InterruptableRawReader.cpp

#include "InterruptableRawReader.h"

bool InterruptableRawReader::openCurrent() {
  QString cmd = "dcraw";
  QStringList args;
  args << "-4" << "-c" << "-w" << current;
  src.start(cmd, args);
  return true;
}

void InterruptableRawReader::stopSource() {
  src.terminate();
}
