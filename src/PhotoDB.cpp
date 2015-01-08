// PhotoDB.cpp

#include "PhotoDB.h"
#include <system_error>
#include <QDebug>
#include "SqlFile.h"
#include <QFile>

PhotoDB::PhotoDB(QString fn): Database(fn) {
  QSqlQuery q(*db);
  if (!q.exec("select id, version from info limit 1") || !q.next()) {
    qDebug() << "PhotoDB: Bad database";
    throw q.lastError();
  }
  qDebug() << "Opened PhotoDB " << fn
	   << ": " << q.value(0).toString() << q.value(1).toString();
}

PhotoDB PhotoDB::create(QString fn) {
  QFile f(fn);
  if (f.exists()) {
    qDebug() << "Could not create new PhotoDB: File exists: " << fn;
    throw std::system_error(std::make_error_code(std::errc::file_exists));
  }
  Database db(fn);
  SqlFile sql(":/setupdb.sql");
  QSqlQuery q(*db);
  db.beginAndLock();
  for (auto c: sql) {
    if (!q.exec(c)) {
      qDebug() << "PhotoDB: Could not setup: " << q.lastError().text();
      qDebug() << "  at " << c;
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
