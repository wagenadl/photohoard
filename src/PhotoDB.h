// PhotoDB.h

#ifndef PHOTODB_H

#define PHOTODB_H

#include "Database.h"

class PhotoDB: public Database {
public:
  PhotoDB(QString fn);
  static PhotoDB create(QString fn);
private:
  QString rootdir;
};

#endif
