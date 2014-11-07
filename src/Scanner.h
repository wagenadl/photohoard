// Scanner.h

#ifndef SCANNER_H

#define SCANNER_H

#include <QThread>
#include "PhotoDB.h"
#include <QMutex>
#include <QWaitCondition>

class Scanner: public QThread {
  Q_OBJECT;
public:
  Scanner(PhotoDB const &);
  virtual ~Scanner();
public slots:
  void start();
  void stop();
  void addTree(QString path);
  void removeTree(QString path);
signals:
  void progressed(int n, int N);
  void done();
protected:
  virtual void run() override;
private:
  quint64 addFolder(quint64 parentid, QString path, QString leaf);
  quint64 addPhoto(quint64 parentid, QString leaf);
  bool findFoldersToScan();
  bool findPhotosToScan();
  void scanFolder(quint64 id);
  void scanPhoto(quint64 id);
  int photoQueueLength();
private:
  PhotoDB db;  
  bool stopsoon;
  QMutex mutex;
  QWaitCondition waiter;
  QMap<QString, quint64> exts;
  int n, N;
};

#endif
