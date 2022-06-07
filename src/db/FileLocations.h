// FileLocations.h

#ifndef FILELOCATIONS_H

#define FILELOCATIONS_H

#include <QString>

namespace FileLocations {
  QString cacheRoot();
  QString dataRoot();
  QString defaultDBFile();
  QString cacheDirForDB(QString dbfn);
  QString sessionFileForDB(QString dbfn);
  void ensureDataRoot();
  void ensureCacheRoot();
};

#endif
