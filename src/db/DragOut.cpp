// DragOut.cpp

#include "DragOut.h"
#include "Exporter.h"
#include <QFileInfo>
#include <QDebug>
#include <QEventLoop>

DragOut::DragOut(SessionDB *db, Exporter *expo,
                 quint64 id, QString fn, QObject *parent):
  QObject(parent), fn(fn), iscomplete(false) {
  /* We make our own exporter, because this is a time-critical job:
     The user is in mid-drag.
     Optimally, we should perhaps suspend other experter threads while
     this is going on.
  */
  exporter = new Exporter(db, this);
  if (expo) 
    exporter->setup(expo->settings());
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
