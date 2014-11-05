// ThreadedCache.h

#ifndef THREADEDCACHE_H

#define THREADEDCACHE_H

class ThreadedCache: public QObject {
public:
  ThreadedCache(QDir root);
  void add(uint64_t id, QImage img);
  void remove(uint64_t id);
  virtual bool request(uint64_t id, QSize size);
  virtual QSize requestPreliminary(uint64_t id, QSize size);
  QImage get(uint64_t id, QSize size);
  void dropRequest(uint64_t id, QSize size);
signals:
  void ready(uint64_t, QSize);
  void preliminary(uint64_t, QSize);
};

#endif
