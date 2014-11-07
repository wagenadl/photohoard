// FolderScanner.h

#ifndef FOLDERSCANNER_H

#define FOLDERSCANNER_H

#include <QThread>
#include <QSqlDatabase>
#include "PhotoScanner.h"
#include <QMutex>
#include <QWaitCondition>
#include <QPointer>
#include <QQueue>
#include <QMap>

class FolderScanner: public QThread {
public:
  FolderScanner(QSqlDatabase const &db, PhotoScanner *phscan);
  ~FolderScanner();
  void start();
  void stop();
  void rescan(quint64 folder, bool internal=false);
  void add(QString dirpath, bool internal=false);
protected:
  virtual void run() override;
private:
  void doscan(quint64 folder);
private:
  QSqlDatabase db;
  QPointer<PhotoScanner> phscan;
  QMutex mutex;
  QWaitCondition waiter;
  bool stopsoon;
};

#endif
