// SessionDB.cpp

#include "SessionDB.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include "PDebug.h"
#include "SqlFile.h"

QString SessionDB::photohoardBaseDir() {
  static QString home = QString(qgetenv("HOME"));
  return home + "/.local/share/photohoard";
}

void SessionDB::ensureBaseDirExists() {
  QDir dir(photohoardBaseDir());
  if (!dir.exists())
    dir.mkpath(".");
}

QString SessionDB::sessionFilename(QString photodbfn) {
  QString leaf = QFileInfo(photodbfn).baseName();
  return photohoardBaseDir() + "/" + leaf + "-session-"
    + QString::number(qHash(photodbfn), 16) + ".db";
}

QString SessionDB::defaultPDBFilename() {
  return photohoardBaseDir() + "/default.db";
}

bool SessionDB::sessionExists(QString photodbfn) {
  return QFileInfo(sessionFilename(photodbfn)).exists();
}

bool SessionDB::isDBReadOnly(QString photodbfn) {
  return !QFileInfo(photodbfn).isWritable();
}


void SessionDB::createSession(QString photodbfn) {
  ensureBaseDirExists();

  QString sessionfn = sessionFilename(photodbfn);
  if (sessionExists(photodbfn))
    CRASH("Could not create new session: File exists: " + sessionfn);

  Database sdb;
  sdb.open(sessionfn);
  SqlFile sql(":/setupsession.sql");
  {
    Transaction t(&sdb);
    for (auto c: sql) 
      sdb.query(c);
    sdb.query("insert into photodb values (:a)", photodbfn);
    t.commit();
  }
  sdb.close();
}

QString SessionDB::photoDBFilename() const {
  QString fn = simpleQuery("select fn from photodb").toString();
  return QFileInfo(fn).canonicalFilePath();
}

SessionDB::SessionDB() {
}

SessionDB::~SessionDB() {
}

void SessionDB::open(QString photodbfn, bool forcereadonly) {
  photodbfn = QFileInfo(photodbfn).canonicalFilePath();
  
  if (!sessionExists(photodbfn))
    CRASH("Cannot open nonexistent session");
  Database::open(sessionFilename(photodbfn));

  {
    QSqlQuery q = query("select id, version from sinfo limit 1");
    ASSERT(q.next());
    if (q.value(0).toString() != "PhotohoardSessionDB")
      CRASH("Cannot open session “" + sessionFilename(photodbfn) + "”.");
  }
  
  query("pragma synchronous = 0");
  query("pragma foreign_keys = on");


  QString codedfn = photoDBFilename();
  if (codedfn != photodbfn) {
    CRASH("Session appears to be meant for different database");
  }
  
  if (!QFileInfo(photodbfn).exists()) {
    CRASH("Cannot open database “" + photodbfn + "”.");
  }
  
  query("attach database :a as P", photodbfn);
  if (forcereadonly || isDBReadOnly(photodbfn)) 
    setReadOnly();
  else
    upgradeDBVersion();
  
  query("attach database ':memory:' as M");
  query("create table if not exists M.filter"
        " ( version integer, "
        "   photo integer )");
  query("create index if not exists M.photoidx on filter(photo)");
  
  query("create table if not exists M.selection"
        " ( version integer unique on conflict ignore )");
}
  

void SessionDB::clone(SessionDB const &src) {
  PhotoDB::clone(src);
  
  query("pragma synchronous = 0");
  query("pragma foreign_keys = on");

  QString pdbfn = simpleQuery("select fn from photodb").toString();
  query("attach database :a as P", pdbfn);
  upgradeDBVersion();
}


void SessionDB::setCurrent(quint64 vsn) {
  if (vsn>0)
    query("update currentvsn set version=:a", vsn);
  else
    query("update currentvsn set version=null");
}

quint64 SessionDB::current() const {
  quint64 vsn = simpleQuery("select version from currentvsn").toULongLong();
  if (!vsn)
    return 0;
  /* I don't *know* if it is important that current() only returns existing
     versions. But it may very well be. I'll have to check that at some
     point and clarify policy.
  */
  QSqlQuery q = constQuery("select id from versions where id==:a limit 1", vsn);
  if (q.next())
    return vsn;
  else
    return 0;
}

void SessionDB::upgradeDBVersion() {
  // update to version 1.2 if needed
  QString dbvsn
    = simpleQuery("select version from info where id=='PhotoDB'").toString();
  // pDebug() << "db version is" << dbvsn;
  if (dbvsn<"1.2") {
    query("alter table layers add column alpha real");
    query("alter table layers add column feather real");
    query("alter table layers add column name text");
    query("update info set version = '1.2' where id=='PhotoDB'");
  }
}
