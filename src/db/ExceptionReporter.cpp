// ExceptionReporter.cpp

#include "ExceptionReporter.h"
#include <QApplication>
#include "PDebug.h"

ExceptionReporter::ExceptionReporter(QObject *parent): QObject(parent) {
  setObjectName("ExceptionReporter");
}

void ExceptionReporter::report(QString s) {
  pDebug() << "ExceptionReporter: " << s;
  pDebug() << "Quitting application";
  QApplication::quit();
}
