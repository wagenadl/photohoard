// InterruptableRawReader.cpp

#include "InterruptableRawReader.h"
#include "PDebug.h"
#include <QProcess>

InterruptableRawReader::InterruptableRawReader(QObject *parent):
  InterruptableReader(parent) {
  src = 0;
}

InterruptableRawReader::~InterruptableRawReader() {
  if (src)
    delete src;
}

void InterruptableRawReader::lPrepSource(QString f, QSize rq, QSize ori) {
  if (src)
    delete src;
  src = new QProcess();
  fn = f;
  rqs = rq;
  oris = ori;
}

void InterruptableRawReader::lUnprepSource() {
  if (src)
    delete src;
  src = 0;
}

QIODevice &InterruptableRawReader::tSource() {
  Q_ASSERT(src);
  return *src;
}

bool InterruptableRawReader::uOpen() {
  Q_ASSERT(src);

  QString cmd = "dcraw";
  QStringList args;
  if (!rqs.isEmpty() && !oris.isEmpty()
      && rqs.width()*2<=oris.width() && rqs.height()*2<=oris.height())
    args << "-h";
  args << QString("-c -t 0 -w -4 -o 5").split(" ");
  args << fn;

  src->start(cmd, args, QProcess::ReadOnly);
  return src->waitForStarted() && src->waitForFinished();
}

void InterruptableRawReader::abort() {
  if (src)
    src->terminate();
}
