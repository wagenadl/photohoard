// PhotoDB.h

#ifndef PHOTODB_H

#define PHOTODB_H

#include "Database.h"
#include <QDateTime>
#include <QSharedPointer>
#include <QMap>

class PhotoDB: public Database {
public:
  PhotoDB(QString fn);
  static PhotoDB create(QString fn);
  quint64 photoFromVersion(quint64 versionid);
  QDateTime captureDate(quint64 photoid);
  QString ftype(int) const;
  QString folder(quint64);
private:
  QString rootdir;
  QSharedPointer< QMap<quint64, QString> > folders;
  QSharedPointer< QMap<int, QString> > ftypes;
};

#endif
