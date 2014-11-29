// Database.cpp

#include "Database.h"
#include <QDebug>
#include <QSqlError>
#include <system_error>
#include <QSqlQuery>
#include <QMutex>
#include "NoResult.h"

Database::Database(Database const &o) {
  db = o.db;
  ref();
}

Database &Database::operator=(Database const &o) {
  if (this != &o) {
    unref();
    db = o.db;
    ref();
  }
  return *this;
}
  
Database::Database(QString filename) {
  if (databases().contains(filename)) {
    db = databases()[filename];
  } else {
    db = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE", filename));
    db->setDatabaseName(filename);
    if (!db->open()) {
      qDebug() << "Could not open database " << filename;
      throw std::system_error(std::make_error_code(std::errc::no_such_file_or_directory));
    }
    databases()[filename] = db;
    mutexes()[db] = new QMutex;
    refcount()[db] = 0;
    names()[db] = filename;
  }
  ref();
}

Database::~Database() {
  unref();
}

void Database::ref() {
  refcount()[db] ++;
}

void Database::unref() {
  refcount()[db] --;
  if (refcount()[db]==0) {
    delete db;
    QString name = names()[db];
    refcount().remove(db);
    names().remove(db);
    databases().remove(name);
    delete mutexes()[db];
    mutexes().remove(db);
  }
}
  
QMap<QString, QSqlDatabase *> &Database::databases() {
  static QMap<QString, QSqlDatabase *> dbs;
  return dbs;
}

QMap<QSqlDatabase *, int> &Database::refcount() {
  static QMap<QSqlDatabase *, int> refs;
  return refs;
}

QMap<QSqlDatabase *, QString> &Database::names() {
  static QMap<QSqlDatabase *, QString> nms;
  return nms;
}

QMap<QSqlDatabase *, QMutex *> &Database::mutexes() {
  static QMap<QSqlDatabase *, QMutex *> mtx;
  return mtx;
}

void Database::beginAndLock() {
  mutexes()[db]->lock();
  QSqlQuery q(*db);
  if (!q.exec("begin transaction")) {
    mutexes()[db]->unlock();
    throw q.lastError();
  }
}

bool Database::tryBeginAndLock() {
  if (!mutexes()[db]->tryLock())
    return false;
  QSqlQuery q(*db);
  if (!q.exec("begin transaction")) {
    mutexes()[db]->unlock();
    qDebug() << "Database: Could not begin transaction: "
	     << q.lastError().text();
    return false;
  }
  return true;
}

void Database::commitAndUnlock() {
  QSqlQuery q(*db);
  bool ok = q.exec("commit transaction");
  mutexes()[db]->unlock();
  if (!ok)
    throw q.lastError();
}

void Database::rollbackAndUnlock() {
  QSqlQuery q(*db);
  bool ok = q.exec("rollback transaction");
  mutexes()[db]->unlock();
  if (!ok)
    throw q.lastError();
}

//////////////////////////////////////////////////////////////////////

Transaction::Transaction(Database const &db):
  db(db), committed(false) {
  this->db.beginAndLock();
}

void Transaction::commit() {
  db.commitAndUnlock();
  committed = true;
}

Transaction::~Transaction() {
  if (!committed)
    db.rollbackAndUnlock();
}
   
QVariant Database::simpleQuery(QString s) {
  QSqlQuery q = query(s);
  if (!q.next())
    throw NoResult();
  return q.value(0);
}

QVariant Database::simpleQuery(QString s, QVariant a) {
  QSqlQuery q = query(s, a);
  if (!q.next())
    throw NoResult();
  return q.value(0);
}
   
QVariant Database::simpleQuery(QString s, QVariant a, QVariant b) {
  QSqlQuery q = query(s, a, b);
  if (!q.next())
    throw NoResult();
  return q.value(0);
}

QVariant Database::simpleQuery(QString s, QVariant a, QVariant b,
                               QVariant c) {
  QSqlQuery q = query(s, a, b, c);
  if (!q.next())
    throw NoResult();
  return q.value(0);
}
   
QVariant Database::simpleQuery(QString s, QVariant a, QVariant b,
                               QVariant c, QVariant d) {
  QSqlQuery q = query(s, a, b, c, d);
  if (!q.next())
    throw NoResult();
  return q.value(0);
}
   
QSqlQuery Database::query(QString s) {
  QSqlQuery q(*db);
  q.prepare(s);
  if (q.exec())
    return q;
  else
    throw q;
}

QSqlQuery Database::query(QString s, QVariant a) {
  QSqlQuery q(*db);
  q.prepare(s);
  q.bindValue(":a", a);
  if (q.exec())
    return q;
  else
    throw q;
}

QSqlQuery Database::query(QString s, QVariant a, QVariant b) {
  QSqlQuery q(*db);
  q.prepare(s);
  q.bindValue(":a", a);
  q.bindValue(":b", b);
  if (q.exec())
    return q;
  else
    throw q;
}

QSqlQuery Database::query(QString s, QVariant a, QVariant b, QVariant c) {
  QSqlQuery q(*db);
  q.prepare(s);
  q.bindValue(":a", a);
  q.bindValue(":b", b);
  q.bindValue(":c", c);
  if (q.exec())
    return q;
  else
    throw q;
}

QSqlQuery Database::query(QString s, QVariant a, QVariant b, QVariant c,
                          QVariant d) {
  QSqlQuery q(*db);
  q.prepare(s);
  q.bindValue(":a", a);
  q.bindValue(":b", b);
  q.bindValue(":c", c);
  q.bindValue(":d", d);
  if (q.exec())
    return q;
  else
    throw q;
}
