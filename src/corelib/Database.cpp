// Database.cpp

#include "Database.h"
#include "PDebug.h"
#include <QSqlError>
#include <system_error>
#include <QSqlQuery>
#include <QThread> // for debug

static bool execWithRetry(QSqlQuery &q) {
  if (q.exec())
    return true;
  if (q.lastError().type()!=QSqlError::ConnectionError)
    return false;
  /* This is a completely horrible hack that prevents some crashes when
     AC_Worker tries to read from the DB while something else is writing.
     This happens on rare occasions, and I cannot figure out why. */
  pDebug() << "Retrying query " << q.lastQuery() << q.boundValues();
  int n = 0;
  while (n<10) {
    if (q.exec()) {
      pDebug() << "Success retrying query " << q.lastQuery() << q.boundValues();
      return true;
    }
    if (q.lastError().type()!=QSqlError::ConnectionError) {
      pDebug() << "Error retrying query " << q.lastQuery() << q.boundValues();
      return false;
    }
    ++n;
  }
  pDebug() << "Failed retrying query " << q.lastQuery() << q.boundValues();
  return false;
}


Database::Database():
  id(autoid()),
  lock(new QMutex()), //QReadWriteLock::NonRecursive)),
  locked(new void*(0)) {
}

void Database::open(QString filename) {
  if (db.isOpen())
    close();

  db = QSqlDatabase::addDatabase("QSQLITE", id);
  db.setDatabaseName(filename);
  if (!db.open())
    CRASH("Could not open database " + filename);
}

void Database::close() {
  db.close();
  db = QSqlDatabase();
  QSqlDatabase::removeDatabase(id);
}

QString Database::autoid() {
  static int id=0;
  id++;
  return QString("db%1").arg(id);
}

void Database::clone(Database const &src) {
  if (db.isOpen())
    close();
  db = QSqlDatabase::cloneDatabase(src.db, id);
  if (!db.open()) 
    CRASH("Could not open cloned database:" + db.databaseName());
  lock = src.lock;
  locked = src.locked;
}

Database::~Database() {
  if (db.isOpen()) {
    CRASH("Database destructed while still open: " + db.databaseName());
    close();
  }
}

void Database::begin() {
  if (!db.transaction()) 
    CRASHDB("Could not begin transaction");
}

void Database::commit() {
  if (!db.commit())
    CRASHDB("Could not commit transaction");
}

void Database::rollback() {
  if (!db.rollback()) 
    CRASHDB("Could not rollback transaction");
}

QVariant Database::defaultQuery(QString s, QVariant dflt) const {
  QSqlQuery q = constQuery(s);
  return q.next() ? q.value(0) : dflt;
}
 
QVariant Database::defaultQuery(QString s, QVariant a, QVariant dflt) const {
  QSqlQuery q = constQuery(s, a);
  return q.next() ? q.value(0) : dflt;
}
  
QVariant Database::simpleQuery(QString s) const {
  QSqlQuery q = constQuery(s);
  if (!q.next()) {
    qDebug() << "simpleQuery" << s;
    CRASH("No result");
  }
  return q.value(0);
}

QVariant Database::simpleQuery(QString s, QVariant a) const {
  QSqlQuery q = constQuery(s, a);
  if (!q.next()) {
    qDebug() << "simpleQuery" << s;
    CRASH("No result");
  }
  return q.value(0);
}
   
QVariant Database::simpleQuery(QString s, QVariant a, QVariant b) const {
  QSqlQuery q = constQuery(s, a, b);
  if (!q.next()) {
    qDebug() << "simpleQuery" << s;
    CRASH("No result");
  }
  return q.value(0);
}

QVariant Database::simpleQuery(QString s, QVariant a, QVariant b,
                               QVariant c) const {
  QSqlQuery q = constQuery(s, a, b, c);
  if (!q.next()) {
    qDebug() << "simpleQuery" << s;
    CRASH("No result");
  }
  return q.value(0);
}
   
QVariant Database::simpleQuery(QString s, QVariant a, QVariant b,
                               QVariant c, QVariant d) const {
  QSqlQuery q = constQuery(s, a, b, c, d);
  if (!q.next()) {
    qDebug() << "simpleQuery" << s;
    CRASH("No result");
  }
  return q.value(0);
}
   
QSqlQuery Database::query(QString s) {
  // Operationally, query and constQuery are identical.
  // Both are provided for easier understanding of code.
  if (!(*locked))
    pDebug() << "query w/o lock" << s;
  return constQuery(s);
}

QSqlQuery Database::constQuery(QString s) const {
  if (!(*locked))
    pDebug() << "constQuery w/o lock" << s;
  if (debugging())
    pDebug() << "query" << (void*)this << s;
  QSqlQuery q(db);
  q.prepare(s);
  if (!execWithRetry(q))
    CRASHQ(q);
  if (debugging())
    pDebug() << "query" << (void*)this << "executed";
  return q;
}

QSqlQuery Database::query(QString s, QVariant a) {
  return constQuery(s, a);
}

QSqlQuery Database::constQuery(QString s, QVariant a) const {
  if (debugging())
    pDebug() << "query" << (void*)this << s << a;
  QSqlQuery q(db);
  q.prepare(s);
  q.bindValue(":a", a);
  if (!execWithRetry(q))
    CRASHQ(q);
  if (debugging())
    pDebug() << "query" << (void*)this << "executed";
  return q;
}

QSqlQuery Database::query(QString s, QVariant a, QVariant b) {
  return constQuery(s, a, b);
}

QSqlQuery Database::constQuery(QString s, QVariant a, QVariant b) const {
  if (debugging())
    pDebug() << "query" << (void*)this << s << a << b;
  QSqlQuery q(db);
  q.prepare(s);
  q.bindValue(":a", a);
  q.bindValue(":b", b);
  if (!execWithRetry(q))
    CRASHQ(q);
  if (debugging())
    pDebug() << "query" << (void*)this << "executed";
  return q;
}

QSqlQuery Database::query(QString s, QVariant a, QVariant b, QVariant c) {
  return constQuery(s, a, b, c);
}

QSqlQuery Database::constQuery(QString s, QVariant a, QVariant b,
                               QVariant c) const {
  if (debugging())
    pDebug() << "query" << (void*)this << s << a << b << c;
  QSqlQuery q(db);
  q.prepare(s);
  q.bindValue(":a", a);
  q.bindValue(":b", b);
  q.bindValue(":c", c);
  if (!execWithRetry(q))
    CRASHQ(q);
  if (debugging())
    pDebug() << "query" << (void*)this << "executed";
  return q;
}

QSqlQuery Database::query(QString s, QVariant a, QVariant b, QVariant c,
                          QVariant d) {
  return constQuery(s, a, b, c, d);
}

QSqlQuery Database::constQuery(QString s, QVariant a, QVariant b, QVariant c,
                               QVariant d) const {
  if (debugging())
    pDebug() << "query" << (void*)this << s << a << b << c << d;
  QSqlQuery q(db);
  q.prepare(s);
  q.bindValue(":a", a);
  q.bindValue(":b", b);
  q.bindValue(":c", c);
  q.bindValue(":d", d);
  if (!execWithRetry(q))
    CRASHQ(q);
  if (debugging())
    pDebug() << "query" << (void*)this << "executed";
  return q;
}

QSqlQuery Database::query(QString s, QVariant a, QVariant b, QVariant c,
                          QVariant d, QVariant e) {
  return constQuery(s, a, b, c, d, e);
}

QSqlQuery Database::constQuery(QString s, QVariant a, QVariant b, QVariant c,
                               QVariant d, QVariant e) const {
  if (debugging())
    pDebug() << "query" << (void*)this << s << a << b << c << d << e;
  QSqlQuery q(db);
  q.prepare(s);
  q.bindValue(":a", a);
  q.bindValue(":b", b);
  q.bindValue(":c", c);
  q.bindValue(":d", d);
  q.bindValue(":e", e);
  if (!execWithRetry(q))
    CRASHQ(q);
  if (debugging())
    pDebug() << "query" << (void*)this << "executed";
  return q;
}

QSqlQuery Database::query(QString s, QVariant a, QVariant b, QVariant c,
                          QVariant d, QVariant e, QVariant f) {
  return constQuery(s, a, b, c, d, e, f);
}

QSqlQuery Database::constQuery(QString s, QVariant a, QVariant b, QVariant c,
                               QVariant d, QVariant e, QVariant f) const {
  if (debugging())
    pDebug() << "query" << (void*)this << s << a << b << c << d << e << f;
  QSqlQuery q(db);
  q.prepare(s);
  q.bindValue(":a", a);
  q.bindValue(":b", b);
  q.bindValue(":c", c);
  q.bindValue(":d", d);
  q.bindValue(":e", e);
  q.bindValue(":f", f);
  if (!execWithRetry(q))
    CRASHQ(q);
  if (debugging())
    pDebug() << "query" << (void*)this << "executed";
  return q;
}

QSqlQuery Database::query(QString s, QVariant a, QVariant b, QVariant c,
                          QVariant d, QVariant e, QVariant f, QVariant g) {
  return constQuery(s, a, b, c, d, e, f, g);
}

QSqlQuery Database::constQuery(QString s, QVariant a, QVariant b, QVariant c,
                               QVariant d, QVariant e, QVariant f, 
			       QVariant g) const {
  if (debugging())
    pDebug() << "query" << (void*)this << s << a << b << c << d << e << f << g;
  QSqlQuery q(db);
  q.prepare(s);
  q.bindValue(":a", a);
  q.bindValue(":b", b);
  q.bindValue(":c", c);
  q.bindValue(":d", d);
  q.bindValue(":e", e);
  q.bindValue(":f", f);
  q.bindValue(":g", g);
  if (!execWithRetry(q))
    CRASHQ(q);
  if (debugging())
    pDebug() << "query" << (void*)this << "executed";
  return q;
}

QSqlQuery Database::query(QString s, QVariant a, QVariant b, QVariant c,
                          QVariant d, QVariant e, QVariant f, QVariant g,
			  QVariant h) {
  return constQuery(s, a, b, c, d, e, f, g, h);
}

QSqlQuery Database::constQuery(QString s, QVariant a, QVariant b, QVariant c,
                               QVariant d, QVariant e, QVariant f, QVariant g,
			       QVariant h) const {
  if (debugging())
    pDebug() << "query" << (void*)this << s
	     << a << b << c << d << e << f << g << h;
  QSqlQuery q(db);
  q.prepare(s);
  q.bindValue(":a", a);
  q.bindValue(":b", b);
  q.bindValue(":c", c);
  q.bindValue(":d", d);
  q.bindValue(":e", e);
  q.bindValue(":f", f);
  q.bindValue(":g", g);
  q.bindValue(":h", h);
  if (!execWithRetry(q))
    CRASHQ(q);
  if (debugging())
    pDebug() << "query" << (void*)this << "executed";
  return q;
}

QSqlQuery Database::query() {
  if (debugging()) 
    pDebug() << "query " << (void*)this;
  return QSqlQuery(db);
}

void Database::enableDebug() {
  debugging() = true;
}

void Database::disableDebug() {
  debugging() = false;
}

bool &Database::debugging() {
  static bool dbg = false;
  return dbg;
}

//////////////////////////////////////////////////////////////////////

Transaction::Transaction(Database *db): db(db) {
  cmt = false;
  pDebug() << "trans";
  db->lockForWriting();
  pDebug() << "Translock";
  db->begin();
  pDebug() << "transbegun";
}

void Transaction::commit() {
  if (cmt) {
    pDebug() << "double commit";
    return;
  }
  cmt = true;
  db->commit();
  db->unlockForWriting();
}

Transaction::~Transaction() {
  if (!cmt) {
    db->rollback();
    db->unlockForWriting();
  }
}


void Database::lockForReading() const {
  if (lock->tryLock(1000)) {
    if (*locked)
      pDebug() << "RELOCK READ!?" << *locked << QThread::currentThread();
    *locked = QThread::currentThread();
    //pDebug() << "Locked for reading" << *locked;
    return;
  }
  pDebug() << "Trying to lock for reading..." << *locked << QThread::currentThread();;
  int n = 1;
  while (!lock->tryLock(1000)) {
    pDebug() << "Still trying R" << n++;
  }
  if (*locked)
    pDebug() << "RELOCK READ!?" << *locked << QThread::currentThread();;
  *locked = QThread::currentThread();
  //pDebug() << "Locked for reading" << *locked;
  return;
}

void Database::lockForWriting() {
  if (lock->tryLock(1000)) {
    if (*locked)
      pDebug() << "RELOCK WRITE!?"  << *locked << QThread::currentThread();
    *locked =  QThread::currentThread();
    // pDebug() << "Locked for writing" << *locked << calltrace();;
    return;
  }
  pDebug() << "Trying to lock for writing..." << *locked << QThread::currentThread();;
  int n = 1;
  while (!lock->tryLock(1000)) {
    pDebug() << "Still trying W" << n++;
  }
  if (*locked)
    pDebug() << "RELOCK WRITE!?"  << *locked << QThread::currentThread();
  *locked = QThread::currentThread();
  // pDebug() << "Locked for writing" << *locked << calltrace();
  return;
}

void Database::unlockForReading() const {
  // pDebug() << "unlock R" << *locked;
  if (!locked)
    pDebug() << "unlock R while not locked";
  *locked = 0;
  lock->unlock();
}

void Database::unlockForWriting() {
  //  pDebug() << "unlock W" << *locked;
  if (!locked)
    pDebug() << "unlock W while not locked";
  *locked = 0;
  lock->unlock();
}
