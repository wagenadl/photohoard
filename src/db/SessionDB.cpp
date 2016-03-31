// SessionDB.cpp

#include "SessionDB.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include "PDebug.h"
#include "SqlFile.h"

QString SessionDB::photohoardBaseDir() {
  static QString home = QString(qgetenv("HOME"));
  return home + "/.local/photohoard";
}

void SessionDB::ensureBaseDirExists() {
 QDir dir(photohoardBaseDir());
  if (!dir.exists())
    dir.mkpath(".");
}

QString SessionDB::sessionFilename(QString dbfn) {
  QFileInfo fi(dbfn);
  if (!fi.exists())
    CRASH("Cannot determine sesion filename for nonexistent db " + dbfn);
  QString absfn = fi.canonicalFilePath();
  QString hash = QString::number(qHash(absfn), 16);
  return photohoardBaseDir() + "/" + hash + ".sdb";
}

bool SessionDB::sessionExists(QString photodbfn) {
  QFileInfo f(sessionFilename(photodbfn));
  return f.exists();
}

bool SessionDB::isReadOnly(QString photodbfn) {
  QFileInfo f(photodbfn);
  return !f.isWritable();
}

void SessionDB::createSession(QString pdbfn) {
  ensureBaseDirExists();

  QString sessionfn = sessionFilename(pdbfn);
  if (sessionExists(pdbfn))
    CRASH("Could not create new session: File exists: " + sessionfn);

  Database db;
  db.open(sessionfn);
  SqlFile sql(":/setupsession.sql");
  {
    Transaction t(&db);

    for (auto c: sql) 
      db.query(c);

    db.query("insert into photodb values (:a)", pdbfn);

    t.commit();
  }
    
  db.close();
}
  
void SessionDB::open(QString photodbfn) {
  if (!sessionExists(photodbfn))
    CRASH("Cannot open nonexistent session for " + photodbfn);
  QString sessionfn = sessionFilename(photodbfn);

  Database::open(sessionfn);

  { QSqlQuery q = query("select id, version from sinfo limit 1");
    ASSERT(q.next());
    if (q.value(0).toString() != "PhotohoardSessionDB")
      CRASH("Cannot open session for " + photodbfn + ": not a session: "
	    + sessionfn);
  }
  QString pdbfn = simpleQuery("select fn from photodb").toString();
  if (pdbfn != QFileInfo(photodbfn).canonicalFilePath())
    CRASH("Session refers to different photo db");

  query("pragma synchronous = 0");
  query("pragma foreign_keys = on");

  query("attach database :a as P", photodbfn);
  if (isReadOnly(photodbfn))
    setReadOnly();
  
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
