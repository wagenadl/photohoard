// Database.h

#ifndef DATABASE_H

#define DATABASE_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMap>

class Database {
public:
  Database(Database const &);
  Database(QString filename);
  virtual ~Database();
  Database &operator=(Database const &);
  QSqlDatabase &operator*() const { return *db; }
  QSqlDatabase &operator->() const { return *db; }
  bool tryBeginAndLock();
  void beginAndLock();
  void commitAndUnlock();
  void rollbackAndUnlock();
  QString fileName() const { return names()[db]; }
protected:
  QSqlDatabase *db;
private:
  void ref();
  void unref();
private:
  static QMap<QString, QSqlDatabase *> &databases();
  static QMap<QSqlDatabase *, QString> &names();
  static QMap<QSqlDatabase *, int> &refcount();
  static QMap<QSqlDatabase *, class QMutex *> &mutexes();
};

class Transaction {
public:
  Transaction(Database const &db);
  ~Transaction();
  void commit();
private:
  Database db;
  bool committed, rolledback;
};
#endif