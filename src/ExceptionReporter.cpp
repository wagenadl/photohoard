// ExceptionReporter.cpp

#include "ExceptionReporter.h"
#include <QApplication>
#include <QDebug>

ExceptionReporter::ExceptionReporter(QObject *parent): QObject(parent) {
  setObjectName("ExceptionReporter");
}

void ExceptionReporter::report(QString s) {
  qDebug() << s;
  QApplication::quit();
}