// Database.h

#ifndef DATABASE_H

#define DATABASE_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMap>
#include <QVariant>

class Database {
public:
  Database(Database const &);
  Database(QString filename, QString id="");
  virtual ~Database();
  Database &operator=(Database const &);
  QSqlDatabase &operator*() const { return *db; }
  QSqlDatabase &operator->() const { return *db; }
  bool tryBeginAndLock();
  void beginAndLock();
  void commitAndUnlock();
  void rollbackAndUnlock();
  QString fileName() const;
  QString name() const;
public:
  // The following execute the query and return the value(0) from the
  // first result row. They throw an exception if there is no result.
  QVariant simpleQuery(QString s) const;
  QVariant simpleQuery(QString s, QVariant a) const;
  QVariant simpleQuery(QString s, QVariant a, QVariant b) const;
  QVariant simpleQuery(QString s, QVariant a, QVariant b, QVariant c) const;
  QVariant simpleQuery(QString s, QVariant a, QVariant b, QVariant c,
                       QVariant d) const;
  // The following execute the query. Result rows can be obtained by repeatedly
  // calling next(). An exception is thrown if the query cannot execute.
  QSqlQuery query(QString s);
  QSqlQuery query(QString s, QVariant a);
  QSqlQuery query(QString s, QVariant a, QVariant b);
  QSqlQuery query(QString s, QVariant a, QVariant b, QVariant c);
  QSqlQuery query(QString s, QVariant a, QVariant b, QVariant c, QVariant d);
  QSqlQuery query(QString s, QVariant a, QVariant b, QVariant c, QVariant d,
		  QVariant e);
  QSqlQuery query(QString s, QVariant a, QVariant b, QVariant c, QVariant d,
		  QVariant e, QVariant f);
  // Following are the same, except that caller promises not to alter the db
  QSqlQuery constQuery(QString s) const;
  QSqlQuery constQuery(QString s, QVariant a) const;
  QSqlQuery constQuery(QString s, QVariant a, QVariant b) const;
  QSqlQuery constQuery(QString s, QVariant a, QVariant b, QVariant c) const;
  QSqlQuery constQuery(QString s, QVariant a, QVariant b, QVariant c,
                       QVariant d) const;
  QSqlQuery constQuery(QString s, QVariant a, QVariant b, QVariant c,
                       QVariant d, QVariant e) const;
  QSqlQuery constQuery(QString s, QVariant a, QVariant b, QVariant c,
                       QVariant d, QVariant e, QVariant f) const;
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
