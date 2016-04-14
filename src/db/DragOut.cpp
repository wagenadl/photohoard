// DragOut.cpp

#include "DragOut.h"
#include "Exporter.h"
#include <QFileInfo>
#include <QDebug>
#include <QEventLoop>

DragOut::DragOut(SessionDB *db, quint64 id, QString fn, QObject *parent):
  QObject(parent), fn(fn), iscomplete(false) {
  exporter = new Exporter(db, this);
  connect(exporter, SIGNAL(completed(QString,int,int)),
          SLOT(completed()));
  exporter->start();
  exporter->add(id, fn);
}

DragOut::~DragOut() {
  exporter->stop();
}

void DragOut::finish() {
  exporter->stop();
}

void DragOut::cancel() {
  exporter->stop();
  QFile f(fn);
  f.remove();
}

void DragOut::ensureComplete() {
  if (iscomplete)
    return;
  
  QEventLoop el(this);
  while (!iscomplete) {
    el.processEvents(QEventLoop::ExcludeUserInputEvents
                     | QEventLoop::WaitForMoreEvents);
  }
}

void DragOut::completed() {
  iscomplete = true;
}
