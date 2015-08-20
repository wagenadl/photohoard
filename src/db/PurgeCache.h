// PurgeCache.h

#ifndef PURGECACHE_H

#define PURGECACHE_H

#include <QString>

class PhotoDB;

namespace PurgeCache {
  void purge(PhotoDB &photodb, QString cachedir);
};

#endif
