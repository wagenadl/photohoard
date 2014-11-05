// AutoCache.h

#ifndef AUTOCACHE_H

#define AUTOCACHE_H

class AutoCache: public ThreadedCache {
public:
  AutoCache(QDir root);
  virtual bool request(uint64_t id, QSize size) inherit;
  virtual QSize requestPreliminary(uint64_t id, QSize size) inherit;
public slots:
  void refresh(uint64_t id); // mark invalid, but do not drop: it will be replaced. Preliminary can still succeed before replacement complete
signals:
  void requested(uint64_t); // somebody else will have to provide
};

#endif
