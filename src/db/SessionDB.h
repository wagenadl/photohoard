// SessionDB.h

#ifndef SESSIONDB_H

#define SESSIONDB_H

#include "PhotoDB.h"

class SessionDB: public PhotoDB {
public:
  static QString photohoardBaseDir();
  static void ensureBaseDirExists();
  static QString sessionFilename(QString photodbfn);
  static bool sessionExists(QString photodbfn);
  static bool isDBReadOnly(QString photodbfn);
  static void createSession(QString photodbfn);
public:
  SessionDB(QString id=""): PhotoDB(id) { }
  virtual void open(QString photodbfn, bool forcereadonly);
  virtual void clone(SessionDB const &);
public:
  void setCurrent(quint64);
  quint64 current() const;
};

#endif
