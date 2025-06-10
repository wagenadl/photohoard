// SessionDB.cpp

#include "SessionDB.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include "PDebug.h"
#include "SqlFile.h"
#include "FileLocations.h"
#include <QUuid>
#include "config.h"

bool SessionDB::sessionExists(QString photodbfn) {
  return QFileInfo(FileLocations::sessionFileForDB(photodbfn)).exists();
}

bool SessionDB::isDBReadOnly(QString photodbfn) {
  return !QFileInfo(photodbfn).isWritable();
}


void SessionDB::createSession(QString photodbfn, QString cachedir) {
  FileLocations::ensureCacheRoot();

  QString sessionfn = FileLocations::sessionFileForDB(photodbfn);
  if (sessionExists(photodbfn))
    CRASH("Could not create new session: File exists: " + sessionfn);

  Database sdb;
  sdb.open(sessionfn);
  SqlFile sql(":/setupsession.sql");
  { Transaction t(&sdb);
    for (auto c: sql) 
      sdb.query(c);
    sdb.query("insert into paths (photodb, cachedir) values (:a, :b)",
              photodbfn, cachedir);
    sdb.query("insert into sinfo(id, val) values(:a, :b)",
              infoIDName(SInfoID::Photohoard), PHOTOHOARD_VERSION);
    t.commit();
  }
  sdb.close();
  qDebug() << "created session" << photodbfn << sessionfn << cachedir;
}

QString SessionDB::photoDBFilename() const {
  DBReadLock lock(this);
  QString fn = simpleQuery("select photodb from paths").toString();
  return QFileInfo(fn).canonicalFilePath();
}


QString SessionDB::cacheDirname() const {
  DBReadLock lock(this);
  QString fn = simpleQuery("select cachedir from paths").toString();
  //  qDebug() << "session cache" << fn;
  return fn; // canonicalizing would fail if not exist
}

SessionDB::SessionDB() {
}

SessionDB::~SessionDB() {
}

QString SessionDB::sessionFilename() const {
  return sessiondbfn;
}

void SessionDB::open(QString photodbfn, bool forcereadonly) {
  photodbfn = QFileInfo(photodbfn).canonicalFilePath();
  sessiondbfn = FileLocations::sessionFileForDB(photodbfn);
  qDebug() << "photodbfn"<<photodbfn;
  qDebug() << "sessiondbfn"<<sessiondbfn;
  if (!sessionExists(photodbfn))
    CRASH("Cannot open nonexistent session");
  Database::open(sessiondbfn);
  
  { DBWriteLock lock(this);
    query("pragma synchronous = 0");
    query("pragma foreign_keys = on");
  }

  QString codedfn = photoDBFilename();
  if (codedfn == "")
    codedfn = photodbfn;
  if (codedfn != photodbfn && codedfn.replace(".db", ".photohoard") != photodbfn) {
    qDebug() << "coded" << codedfn << codedfn.replace(".db", ".photohoard");
    qDebug() << "photodbfn" << photodbfn;
    CRASH("Session appears to be meant for different database");
  }
  
  if (!QFileInfo(photodbfn).exists()) {
    CRASH("Cannot open database “" + photodbfn + "”.");
  }

  { DBWriteLock lock(this);
    query("attach database :a as P", photodbfn);
  }
  if (forcereadonly || isDBReadOnly(photodbfn)) 
    setReadOnly();
  else
    upgradeDBVersion();


  { DBReadLock lock(this);
    QSqlQuery q = constQuery("select count(1) from sinfo where id==:a",
                             infoIDName(SInfoID::Photohoard));
    if (!q.next())
      CRASH("Cannot open session for “" + photodbfn + "”.");
  }
  
  DBWriteLock lock(this);
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

  { DBWriteLock lock(this);
    query("pragma synchronous = 0");
    query("pragma foreign_keys = on");
    QString pdbfn = simpleQuery("select photodb from paths").toString();
    query("attach database :a as P", pdbfn);
  }
  upgradeDBVersion();
}


void SessionDB::setCurrent(quint64 vsn) {
  DBWriteLock lock(this);
  if (vsn>0)
    query("update currentvsn set version=:a", vsn);
  else
    query("update currentvsn set version=null");
}

quint64 SessionDB::current() const {
  DBReadLock lock(this);
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

QString SessionDB::infoIDName(SessionDB::SInfoID id) {
  static QMap<SInfoID, QString> map = {
    { SInfoID::Photohoard, "photohoard" },
    { SInfoID::SessionDBVersion, "sessiondb" },
    { SInfoID::LastImportCollection, "lastimportcollection" },
    { SInfoID::RunningPID, "runningpid" },
  };
  return map[id];
}

void SessionDB::setSessionDBInfo(SessionDB::SInfoID id, QVariant val) {
  DBWriteLock lock(this);
  query("insert or replace into sinfo(id, val) values(:a,:b)",
        infoIDName(id), val);
}

QVariant SessionDB::sessionDBInfo(SessionDB::SInfoID id) const {
  DBReadLock lock(this);
  auto q = constQuery("select val from sinfo where id==:a", infoIDName(id));
  if (q.next())
    return q.value(0);
  return QVariant();
}

void SessionDB::upgradeDBVersion() {
  bool modern = simpleQuery("select count(1) from sinfo where id==:a",
                            infoIDName(SInfoID::Photohoard)).toInt() > 0;
  if (modern)
    return; /* In the future, we may have to check for version, but for now,
               the only "modern" version in existence is 1.4. */

  /* We have an old style SINFO table. */

  QString sdbvsn = simpleQuery("select version from sinfo where id==:a",
                               "PhotohoardSessionDB").toString();
  
  // update to version 1.3 if needed
  if (sdbvsn < "1.3") {
    Transaction t(this);
    query("alter table sinfo add column runningpid integer");
    query("update sinfo set version='1.3' where id=='PhotohoardSessionDB'");
    t.commit();
  }

  // upgrade to modern style sinfo table
  { Transaction t(this);
    QVariant runningpid = simpleQuery("select runningpid from sinfo"
                                      " where id=='PhotohoardSessionDB'");
    query("drop table sinfo");
    query("create table sinfo ("
          " id text unique,"
          " val )");
    query("insert into sinfo(id, val) values(:a, :b)",
          infoIDName(SInfoID::Photohoard), PHOTOHOARD_VERSION);
    query("insert into sinfo(id, val) values(:a, :b)",
          infoIDName(SInfoID::SessionDBVersion), "1.4");
    query("insert into sinfo(id, val) values(:a, :b)",
          infoIDName(SInfoID::RunningPID), runningpid);
    t.commit();
  }
    
}

void SessionDB::storePid(quint64 pid) {
  setSessionDBInfo(SInfoID::RunningPID, pid);
}

quint64 SessionDB::retrievePid() const {
  return sessionDBInfo(SInfoID::RunningPID).toULongLong();
}
  
