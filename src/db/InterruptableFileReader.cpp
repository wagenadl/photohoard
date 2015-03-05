// InterruptableFileReader.cpp

#include "InterruptableFileReader.h"

bool InterruptableFileReader::openCurrent() {
  src.setFileName(current);
  if (!src.open(QFile::ReadOnly))
    return false;
  size = src.size();
  dest.resize(size);
  return true;
}

qint64 InterruptableFileReader::nextChunkSize() {
  qint64 N0 = InterruptableReader::nextChunkSize();
  qint64 N = size - offset;
  return N<N0 ? N : N0;
}
