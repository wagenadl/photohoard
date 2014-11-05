// BasicCache.h

#ifndef BASICCACHE_H

#define BASICCACHE_H

#include <stdint.h>

class BasicCache {
public:
  BasicCache(QDir root);
  static BasicCache create(QDir root,
                           QSet<int> sizes = BasicCache::defaultSizes());
  static QSet<int> defaultSizes();
  void add(uint64_t id, QImage img);
  void remove(uint64_t id);
  QImage get(uint64_t id, QSize size);
  QSize bestSize(uint64_t id, QSize size);
};

#endif
