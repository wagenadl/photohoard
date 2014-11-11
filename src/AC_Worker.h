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
  void recache(QSet<quint64> versions);
  void handleFoundImage(quint64 id, QImage img);
private:
  PhotoDB db;
  class BasicCache *cache;
  class IF_Bank *bank;
  int n, N;
  QQueue<quint64> readyToLoad;
  QSet<quint64> beingLoaded;
  QSet<quint64> invalidatedWhileLoading;
  QMap<quint64, QImage> loaded;
};

#endif
