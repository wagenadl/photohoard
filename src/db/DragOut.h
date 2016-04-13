// DragOut.h

#ifndef DRAGOUT_H

#define DRAGOUT_H

#include <QObject>

class DragOut: public QObject {
  Q_OBJECT;
public:
  DragOut(class SessionDB *, quint64 id, QString fn, QObject *parent=0);
  virtual ~DragOut();
  void cancel();
  void finish();
  void ensureComplete();
private slots:
  void completed();
private:
  QString fn;
  bool iscomplete;
  class Exporter *exporter;
};

#endif
