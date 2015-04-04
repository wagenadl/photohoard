// Database.cpp

#include "Database.h"
#include "PDebug.h"
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
  
Database::Database(QString filename, QString id) {
  if (id=="")
    id = filename;
  if (databases().contains(id)) {
    db = databases()[id];
  } else {
    db = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE", id));
    db->setDatabaseName(filename);
    if (!db->open()) {
      qDebug() << "Could not open database " << filename;
      throw std::system_error(std::make_error_code(std::errc::no_such_file_or_directory));
    }
    databases()[id] = db;
    mutexes()[db] = new QMutex;
    refcount()[db] = 0;
    names()[db] = id;

    query("attach database ':memory:' as M");
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
   
QVariant Database::simpleQuery(QString s) const {
  QSqlQuery q = constQuery(s);
  if (!q.next())
    throw NoResult(s);
  return q.value(0);
}

QVariant Database::simpleQuery(QString s, QVariant a) const {
  QSqlQuery q = constQuery(s, a);
  if (!q.next())
    throw NoResult(s);
  return q.value(0);
}
   
QVariant Database::simpleQuery(QString s, QVariant a, QVariant b) const {
  QSqlQuery q = constQuery(s, a, b);
  if (!q.next())
    throw NoResult(s);
  return q.value(0);
}

QVariant Database::simpleQuery(QString s, QVariant a, QVariant b,
                               QVariant c) const {
  QSqlQuery q = constQuery(s, a, b, c);
  if (!q.next())
    throw NoResult(s);
  return q.value(0);
}
   
QVariant Database::simpleQuery(QString s, QVariant a, QVariant b,
                               QVariant c, QVariant d) const {
  QSqlQuery q = constQuery(s, a, b, c, d);
  if (!q.next())
    throw NoResult(s);
  return q.value(0);
}
   
QSqlQuery Database::query(QString s) {
  // Operationally, query and constQuery are identical.
  // Both are provided for easier understanding of code.
  return constQuery(s);
}

QSqlQuery Database::constQuery(QString s) const {
  QSqlQuery q(*db);
  q.prepare(s);
  if (q.exec())
    return q;
  else
    throw q;
}

QSqlQuery Database::query(QString s, QVariant a) {
  return constQuery(s, a);
}

QSqlQuery Database::constQuery(QString s, QVariant a) const {
  QSqlQuery q(*db);
  q.prepare(s);
  q.bindValue(":a", a);
  if (q.exec())
    return q;
  else
    throw q;
}

QSqlQuery Database::query(QString s, QVariant a, QVariant b) {
  return constQuery(s, a, b);
}

QSqlQuery Database::constQuery(QString s, QVariant a, QVariant b) const {
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
  return constQuery(s, a, b, c);
}

QSqlQuery Database::constQuery(QString s, QVariant a, QVariant b,
                               QVariant c) const {
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
  return constQuery(s, a, b, c, d);
}

QSqlQuery Database::constQuery(QString s, QVariant a, QVariant b, QVariant c,
                               QVariant d) const {
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

QSqlQuery Database::query(QString s, QVariant a, QVariant b, QVariant c,
                          QVariant d, QVariant e) {
  return constQuery(s, a, b, c, d, e);
}

QSqlQuery Database::constQuery(QString s, QVariant a, QVariant b, QVariant c,
                               QVariant d, QVariant e) const {
  QSqlQuery q(*db);
  q.prepare(s);
  q.bindValue(":a", a);
  q.bindValue(":b", b);
  q.bindValue(":c", c);
  q.bindValue(":d", d);
  q.bindValue(":e", e);
  if (q.exec())
    return q;
  else
    throw q;
}

QSqlQuery Database::query(QString s, QVariant a, QVariant b, QVariant c,
                          QVariant d, QVariant e, QVariant f) {
  return constQuery(s, a, b, c, d, e, f);
}

QSqlQuery Database::constQuery(QString s, QVariant a, QVariant b, QVariant c,
                               QVariant d, QVariant e, QVariant f) const {
  QSqlQuery q(*db);
  q.prepare(s);
  q.bindValue(":a", a);
  q.bindValue(":b", b);
  q.bindValue(":c", c);
  q.bindValue(":d", d);
  q.bindValue(":e", e);
  q.bindValue(":f", f);
  if (q.exec())
    return q;
  else
    throw q;
}

QString Database::fileName() const {
  return db->databaseName();
}

QString Database::name() const {
  return db->connectionName();
}
