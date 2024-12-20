// SessionDB.h

#ifndef SESSIONDB_H

#define SESSIONDB_H

#include "PhotoDB.h"

class SessionDB: public PhotoDB {
public:
  enum class SInfoID {
    Photohoard,
    SessionDBVersion,
    LastImportCollection,
    RunningPID
  };
public:
  static bool sessionExists(QString photodbfn);
  static void createSession(QString photodbfn, QString cachedir);
  static bool isDBReadOnly(QString photodbfn);
public:
  SessionDB();
  virtual ~SessionDB();
  QString photoDBFilename() const;
  QString sessionFilename() const;
  QString cacheDirname() const;
  virtual void open(QString photodbfn, bool forcereadonly);
  virtual void clone(SessionDB const &);
  quint64 retrievePid() const;
  void storePid(quint64);
  QVariant sessionDBInfo(SInfoID id) const;
  void setSessionDBInfo(SInfoID id, QVariant val);
public:
  void setCurrent(quint64);
  quint64 current() const;
private:
  static QString infoIDName(SInfoID);
  void upgradeDBVersion();
private:
  QString sessiondbfn;
};

#endif
