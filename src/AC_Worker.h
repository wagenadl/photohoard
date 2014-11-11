// AC_Worker.h

#ifndef AC_WORKER_H

#define AC_WORKER_H

#include <QObject>
#include "PhotoDB.h"
#include <QSet>
#include <QImage>

class AC_Worker: public QObject {
  Q_OBJECT;
public:
  AC_Worker(PhotoDB const &db, class BasicCache *cache,
            QObject *parent=0);
public slots:
  void boot(); // get initial list of cachable items from db
  void recache(QSet<quint64> versions);
  void handleFoundImage(quint64 id, QImage img);
signals:
  void cacheProgress(int n, int N);
  void doneCaching();
private:
  QSet<quint64> getSomeFromDBQueue(int maxres);
  void markReadyToLoad(QSet<quint64> versions);  
  void addToDBQueue(QSet<quint64> versions);
  void activateBank();
private:
  PhotoDB db;
  class BasicCache *cache;
  class IF_Bank *bank;
  int n, N;
  QQueue<quint64> readyToLoad;
  QSet<quint64> rtlSet;
  QSet<quint64> beingLoaded;
  QSet<quint64> invalidatedWhileLoading;
  QMap<quint64, QImage> loaded;
};

#endif
