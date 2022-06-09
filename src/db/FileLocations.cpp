// FileLocations.cpp

#include "FileLocations.h"

#include <QDir>
#include <QFileInfo>
#include "Settings.h"

namespace FileLocations {
  Settings settings;
  
  QString cacheRoot() {
    return settings.get("cacheroot",
                        QDir::homePath() + "/.cache/photohoard")
      .toString();
  }

  QString dataRoot() {
    return settings.get("dataroot",
                        QDir::homePath() + "/.local/share/photohoard")
      .toString();
  }

  QString defaultDBFile() {
    return dataRoot() + "/default.db";
  }

  QString shortDBName(QString fn) {
    fn = QFileInfo(fn).canonicalFilePath();
    if (fn.endsWith(".db"))
      fn = fn.left(fn.size() - 3);
    if (fn.startsWith(dataRoot() + "/"))
      fn = fn.mid((dataRoot() + "/").size());
    return fn.replace("/", "_");
  }
  
  QString cacheDirForDB(QString fn) {
    return cacheRoot() + "/" + shortDBName(fn) + "-cache";
  }

  QString sessionFileForDB(QString fn) {
    return cacheRoot() + "/" + shortDBName(fn) + "-session.db";
  }

  void ensureDataRoot() {
    QDir dir(dataRoot());
    if (!dir.exists())
      dir.mkpath(".");
  }
  
  void ensureCacheRoot() {
    QDir dir(cacheRoot());
    if (!dir.exists())
      dir.mkpath(".");
  }
};

