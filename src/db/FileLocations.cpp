// FileLocations.cpp

#include "FileLocations.h"

#include <QDir>
#include <QFileInfo>
#include "Settings.h"
#include "PhotoDB.h"
#include "SessionDB.h"

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
    return dataRoot() + "/default.photohoard";
  }

  QString defaultImageRoot() {
    return QDir::home().absoluteFilePath("Pictures");
  }

  QString databaseUuid(QString dbfn) {
    //// this mechanism does not yet work, because we do not yet store uuids 
    //QStringList uuidlist
    //  = Settings().get("uuids", QQstringList()).toStringList();
    //for (int k=0; k<uuidlist.size()-1; k+=2)
    //  if (uuidlist[k]==dbfn)
    //    return uuidlist[k+1];

    // this mechanism should now work
    PhotoDB pdb;
    pdb.open(dbfn);
    QString id = pdb.databaseID();
    pdb.close();
    return id;
  }
  
  QString defaultCacheDirForDB(QString fn) {
    QString uuid = databaseUuid(fn);
    return cacheRoot() + "/" + uuid + "-cache";
  }

  QString sessionFileForDB(QString fn) {
    QString uuid = databaseUuid(fn);
    return cacheRoot() + "/" + uuid + "-session.db";
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

