// AC_Worker.h

#ifndef AC_WORKER_H

#define AC_WORKER_H

#include <QObject>

#include <QSet>
#include "Image16.h"
#include <QMap>
#include "PhotoDB.h"

class AC_Worker: public QObject {
  Q_OBJECT;
public:
  AC_Worker(PhotoDB const *db, QString rootdir,
	    class AC_ImageHolder *holder,
            QObject *parent=0);
  virtual ~AC_Worker();
public slots:
  void boot(); // get initial list of cachable items from db
  void recache(QSet<quint64> versions);
  void requestImage(quint64 version, QSize desired);
  void requestIfEasy(quint64 version, QSize desired);
  void cachePreview(quint64 vsn, Image16 img);
  void cacheModified(quint64 vsn);
private slots:
  void handleFoundImage(quint64 version, Image16 img, QSize fullSize);
signals:
  void cacheProgress(int n, int N);
  void doneCaching();
  void available(quint64 version, QSize requested, Image16 img);
  void exception(QString);
private:
  void respondToRequest(quint64 version, Image16 img);
  QSet<quint64> getSomeFromDBQueue(int maxres=100);
  void markReadyToLoad(QSet<quint64> versions);  
  void addToDBQueue(QSet<quint64> versions);
  void activateBank();
  void sendToBank(quint64 version);
  void storeLoadedInDB();
  void ensureDBSizeCorrect(quint64 vsn, PSize);
  void countQueue();
  int queueLength();
  void processLoaded();
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
  QMap<quint64, Image16> loaded;
  QSet<quint64> outdatedLoaded;
  QMap<quint64, QSet<PSize> > requests;
  class AC_ImageHolder *holder;
};

#endif
