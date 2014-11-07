// PhotoScanner.h

#ifndef PHOTOSCANNER_H

#define PHOTOSCANNER_H

#include <QThread>
#include <QSqlDatabase>
#include <QMutex>
#include <QWaitCondition>
#include <QMap>

class PhotoScanner: public QThread {
public:
  PhotoScanner(QSqlDatabase const &db, class AutoCache *cache=0);
  ~PhotoScanner();
  void start();
  void stop();
  void rescan(quint64 photo, bool internal=false);
  bool add(quint64 folder, QString leafname, bool internal=false);
protected:
  virtual void run() override;
private:
  void doscan(quint64 photo);
private:
  QSqlDatabase db;
  bool stopsoon;
  QMutex mutex;
  QWaitCondition waiter;
  QMap<QString, quint64> exts;
  // QPointer<AutoCache> cache;
};

#endif
