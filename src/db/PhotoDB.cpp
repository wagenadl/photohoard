// PhotoDB.cpp

#include "PhotoDB.h"
#include <system_error>
#include "PDebug.h"
#include "SqlFile.h"
#include <QFile>
#include "NoResult.h"

PhotoDB::PhotoDB(QString fn): Database(fn),
                              folders(new QMap<quint64, QString>),
                              ftypes(new QMap<int, QString>) {
  QSqlQuery q = query("select id, version from info limit 1");
  if (!q.next())
    throw NoResult();

  pDebug() << "Opened PhotoDB " << fn
	   << ": " << q.value(0).toString() << q.value(1).toString();

  query("pragma synchronous = 0");

  q = query("select id, stdext from filetypes");
  while (q.next()) 
    (*ftypes)[q.value(0).toInt()] = q.value(1).toString();
}

QString PhotoDB::ftype(int ft) const {
  return (*ftypes)[ft];
}

QString PhotoDB::folder(quint64 id) {
  if (folders->contains(id))
    return (*folders)[id];

  QString folder = simpleQuery("select pathname from folders where id=:i", id)
    .toString();
  (*folders)[id] = folder;
  return folder;
}

PhotoDB PhotoDB::create(QString fn) {
  QFile f(fn);
  if (f.exists()) {
    pDebug() << "Could not create new PhotoDB: File exists: " << fn;
    throw std::system_error(std::make_error_code(std::errc::file_exists));
  }
  Database db(fn);
  SqlFile sql(":/setupdb.sql");
  QSqlQuery q(*db);
  db.beginAndLock();
  for (auto c: sql) {
    if (!q.exec(c)) {
      pDebug() << "PhotoDB: Could not setup: " << q.lastError().text();
      pDebug() << "  at " << c;
      db.rollbackAndUnlock();
      throw q;
    }
  }
  db.commitAndUnlock();
  return PhotoDB(fn);
}

quint64 PhotoDB::photoFromVersion(quint64 v) {
  return simpleQuery("select photo from versions where id==:a limit 1", v)
    .toULongLong();
}

QDateTime PhotoDB::captureDate(quint64 p) {
  return simpleQuery("select capturedate from photos where id==:a", p)
    .toDateTime();
}
