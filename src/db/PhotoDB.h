// PhotoDB.h

#ifndef PHOTODB_H

#define PHOTODB_H

#include "Database.h"
#include <QDateTime>

class PhotoDB: public Database {
public:
  PhotoDB(QString fn);
  static PhotoDB create(QString fn);
  quint64 photoFromVersion(quint64 versionid);
  QDateTime captureDate(quint64 photoid);
private:
  QString rootdir;
};

#endif
