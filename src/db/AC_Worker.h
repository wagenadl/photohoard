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
  void requestImage(quint64 version, QSize desired);
  void cachePreview(quint64 vsn, QImage img);
private slots:
  void handleFoundImage(quint64 id, QImage img, bool isFullSize);
signals:
  void cacheProgress(int n, int N);
  void doneCaching();
  void available(quint64 version, QSize requested, QImage img);
  void exception(QString);
private:
  void respondToRequest(quint64 version, QImage img);
  QSet<quint64> getSomeFromDBQueue(int maxres=100);
  void markReadyToLoad(QSet<quint64> versions);  
  void addToDBQueue(QSet<quint64> versions);
  void activateBank();
  void sendToBank(quint64 version);
  void storeLoadedInDB();
  void readFTypes();
  void ensureDBSizeOK(quint64 vsn, QSize);
  void countQueue();
  int queueLength();
private:
  PhotoDB db;
  class BasicCache *cache;
  class IF_Bank *bank;
  int n, N;
  int threshold;
  QSet<quint64> readyToLoad;
  QList<quint64> rtlOrder;
  QSet<quint64> mustCache;
  QSet<quint64> beingLoaded;
  QSet<quint64> invalidatedWhileLoading;
  QMap<quint64, QImage> loaded;
  QSet<quint64> outdatedLoaded;
  QMap<quint64, QString> folders;
  QMap<quint64, QSet<QSize> > requests;
  QMap<int, QString> ftypes;
};

#endif