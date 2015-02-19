// ExceptionReporter.h

#ifndef EXCEPTIONREPORTER_H

#define EXCEPTIONREPORTER_H

#include <QObject>

class ExceptionReporter: public QObject {
  Q_OBJECT;
public:
  ExceptionReporter(QObject *parent=0);
public slots:
  void report(QString);
};

#endif
