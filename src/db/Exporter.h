// Exporter.h

#ifndef EXPORTER_H

#define EXPORTER_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QSet>
#include <QMap>

#include "SessionDB.h"
#include "ExportSettings.h"

class Exporter: public QThread {
  Q_OBJECT;
public:
  class Job {
  public:
    ExportSettings settings;
    QSet<quint64> todo;
    QSet<quint64> completed;
    QSet<quint64> failed;
  };
public:
  Exporter(SessionDB *db, QObject *parent=0);
  void setup(ExportSettings const &);
  void addSelection();
  void add(QSet<quint64> const &vsns);
  void start();
  void stop();
  virtual ~Exporter();
signals:
  void progress(int n, int N);
  void completed(QString dest, int nOK, int nFailures);
protected:
  virtual void run();
private:
  bool doExport(quint64 vsn, ExportSettings const &settings);
private:
  SessionDB *db0; // calling thread
  QMutex mutex;
  QWaitCondition cond;
  ExportSettings settings;
  SessionDB db; // our clone
  QList<Job> jobs;
  bool stopsoon;
  class IF_Worker *worker;
};


#endif
