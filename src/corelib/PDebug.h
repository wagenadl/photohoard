// PDebug.h

#ifndef PDEBUG_H

#define PDEBUG_H

#include <QDebug>

QDebug pDebug();

namespace PDebug {
  void reset();
  QTime &time();
}

inline void a_nop() { }

QString calltrace();
void complain(QString msg, char const *file, int line);
void crash(QString msg, char const *file, int line);
void crashdb(class QSqlDatabase const &db, QString msg,
             char const *file, int line);
void crashq(class QSqlQuery const &q, char const *file, int line);

#define CRASH(msg) crash(msg, __FILE__, __LINE__)

#define COMPLAIN(msg) complain(msg, __FILE__, __LINE__)

#define CRASHDB(msg) crashdb(db, msg, __FILE__, __LINE__)

#define CRASHQ(q) crashq(q, __FILE__, __LINE__)

#define ASSERT(cond) ((!(cond)) ? crash(#cond, __FILE__, __LINE__) : a_nop())

#endif
