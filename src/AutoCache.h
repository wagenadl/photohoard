// AutoCache.h

#ifndef AUTOCACHE_H

#define AUTOCACHE_H

#include <QObject>
#include <QSet>
#include "PhotoDB.h"

class AutoCache: public QObject {
  Q_OBJECT;
public:
  AutoCache(PhotoDB const &db, QString rootdir, QObject *parent=0);
  virtual ~AutoCache();
public slots:
  void recache(QSet<quint64> ids);
  void recache(quint64 id);


signals: // private
  void forwardRecache(QSet<quint64> ids);
private:
  QThread thread;
  class AC_Worker *worker;
  PhotoDB db;
  class BasicCache *cache;
};

#endif
