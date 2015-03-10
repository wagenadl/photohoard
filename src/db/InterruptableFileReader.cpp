// InterruptableFileReader.cpp

#include "InterruptableFileReader.h"
#include <QDebug>
#include <QFile>

InterruptableFileReader::InterruptableFileReader(QObject *parent):
  InterruptableReader(parent) {
  src = 0;
}

InterruptableFileReader::~InterruptableFileReader() {
  if (src)
    delete src;
}

QIODevice &InterruptableFileReader::tSource() {
  Q_ASSERT(src);
  return *src;
}

void InterruptableFileReader::lPrepSource(QString fn, QSize, QSize) {
  if (src)
    delete src;
  src = new QFile(fn);
}

void InterruptableFileReader::lUnprepSource() {
  if (src)
    delete src;
  src = 0;
}

bool InterruptableFileReader::uOpen() {
  Q_ASSERT(src);
  return src->open(QFile::ReadOnly);
}

int InterruptableFileReader::uEstimateSize() {
  return src ? src->size() : 0;
}
