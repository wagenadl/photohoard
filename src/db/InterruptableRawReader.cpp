// InterruptableRawReader.cpp

#include "InterruptableRawReader.h"
#include <QDebug>

InterruptableRawReader::InterruptableRawReader(QObject *parent):
  InterruptableReader(parent) {
  src = new QProcess(this);
}

bool InterruptableRawReader::open() {
  QString cmd = "dcraw";
  QStringList args;
  args << "-c" << "-w" << requested;
  // eventually we should return 16-bits linear XYZ!
  qDebug() << cmd << args;
  src->start(cmd, args, QProcess::ReadOnly);
  return src->waitForStarted();
}

void InterruptableRawReader::abort() {
  qDebug() << "IRR: stopSource";
  src->terminate();
}
