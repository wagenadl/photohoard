// InterruptableFileReader.cpp

#include "InterruptableFileReader.h"
#include <QDebug>

InterruptableFileReader::InterruptableFileReader(QObject *parent):
  InterruptableReader(parent) {
  src = new QFile(this);
  connect(src, SIGNAL(readyRead()), SLOT(readSome()));
}

bool InterruptableFileReader::open() {
  qDebug() << "IFR::openCurrent" << requested;
  src->setFileName(requested);
  if (!src->open(QFile::ReadOnly))
    return false;
  dest.resize(src->size());
  return true;
}

