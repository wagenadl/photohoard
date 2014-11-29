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
public:
  QVariant simpleQuery(QString s);
  QVariant simpleQuery(QString s, QVariant a);
  QVariant simpleQuery(QString s, QVariant a, QVariant b);
  QVariant simpleQuery(QString s, QVariant a, QVariant b, QVariant c);
  QVariant simpleQuery(QString s, QVariant a, QVariant b, QVariant c,
                       QVariant d);
  QSqlQuery query(QString s);
  QSqlQuery query(QString s, QVariant a);
  QSqlQuery query(QString s, QVariant a, QVariant b);
  QSqlQuery query(QString s, QVariant a, QVariant b, QVariant c);
  QSqlQuery query(QString s, QVariant a, QVariant b, QVariant c, QVariant d);
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
