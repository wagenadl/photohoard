// PDebug.cpp

#include "PDebug.h"
#include <QTime>
#include <execinfo.h>
#include <cxxabi.h>
#include <QStringList>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

namespace PDebug {
  QTime &time() {
    static QTime t0;
    static bool started = false;
    if (!started) {
      t0.start();
      started = true;
    }
    return t0;
  }

  void reset() {
    time().restart();
  }
};

QDebug pDebug() {
  return qDebug() << QString("%1").arg(PDebug::time().elapsed()/1000.0,
                                       7, 'f', 3).toUtf8().data();
}

QStringList calltrace_list() {
  QStringList fns;
  void *backtrace_data[1024];
  int backtrace_count = backtrace(backtrace_data, 1024);
  char **symbols = backtrace_symbols(backtrace_data, backtrace_count);
  size_t dm_length = 1024;
  char demangled[1024];
  for (int k=1; k<backtrace_count; k++) {
    int status;
    char *begin_name=0, *begin_offset=0, *end_offset=0;
    for (char *p=symbols[k]; *p; ++p) {
      if (*p=='(')
        begin_name=p;
      else if (*p=='+')
        begin_offset=p;
      else if (*p==')' && begin_offset && !end_offset)
        end_offset=p;
    }
    if (begin_name && begin_offset && end_offset
        && begin_name<begin_offset) {
      *begin_name++ = *begin_offset++ = *end_offset = 0;
      abi::__cxa_demangle(begin_name, demangled, &dm_length, &status);
      if (status==0) 
        fns << demangled;
      else 
        fns << begin_name;
    } else {
      fns << ""; // don't try to use a useless symbol
    }
  }
    
  free(symbols);
  return fns;
}


QString calltrace() {
  QStringList fns = calltrace_list();
  fns.pop_front();
  fns.pop_front();
  QString trc = fns.join(" < ");
  trc.replace(QRegExp(" <  <  < ( < )*"), " <...< ");
  trc.replace("  ", " ");
  return trc;
}





void complain(QString msg, char const *file, int line) {
  qDebug() << "Trouble. Calltrace follows.";
  qDebug() << calltrace();
  qDebug() << "Location:" << file << line;
  qDebug() << "Message:" << msg;
  abort();
}

void crash(QString msg, char const *file, int line) {
  qDebug() << "Fatal error. Calltrace follows.";
  qDebug() << calltrace();
  qDebug() << "Location:" << file << line;
  qDebug() << "Message:" << msg;
  abort();
}

void crashdb(QSqlDatabase const &db, QString msg, char const *file, int line) {
  msg += ": " + db.lastError().databaseText()
    + " (" + db.lastError().driverText() + ")";
  crash(msg, file, line);
}

void crashq(QSqlQuery const &q, char const *file, int line) {
  QString msg = q.lastQuery() + ": " + q.lastError().databaseText()
    + " (" + q.lastError().driverText() + ")";
  crash(msg, file, line);
}
