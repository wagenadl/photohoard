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
    QMap<quint64, QString> fnoverride;
  };
public:
  Exporter(SessionDB *db, QObject *parent=0);
  void setup(ExportSettings const &);
  ExportSettings const &settings() const { return settings_; }
  void addSelection();
  void sendEmail();
  void add(QSet<quint64> const &vsns);
  void add(quint64 vsn, QString ofn);
  void start();
  void stop();
  virtual ~Exporter();
signals:
  void progress(int n, int N);
  void completed(QString dest, int nOK, int nFailures);
protected:
  virtual void run();
private:
  void copyFilenameToClipboard(quint64 vsn);
  QString doExport(quint64 vsn, ExportSettings const &settings,
                   QString ovr=QString()); // returns output filename or "" if failed
private:
  SessionDB *db0; // calling thread
  QMutex mutex;
  QWaitCondition cond;
  ExportSettings settings_;
  SessionDB db; // our clone
  QList<Job> jobs;
  bool stopsoon;
  class IF_Worker *worker;
  QSet<QString> emailready;
};


#endif
