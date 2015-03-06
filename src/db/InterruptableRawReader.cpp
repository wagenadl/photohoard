// InterruptableRawReader.cpp

#include "InterruptableRawReader.h"

bool InterruptableRawReader::openCurrent() {
  QString cmd = "dcraw";
  QStringList args;
  args << "-c" << "-w" << current;
  // eventually we should return 16-bits linear XYZ!
  src.start(cmd, args);
  return true;
}

void InterruptableRawReader::stopSource() {
  src.terminate();
}

bool InterruptableRawReader::atEnd() const {
  return src.atEnd();
}
