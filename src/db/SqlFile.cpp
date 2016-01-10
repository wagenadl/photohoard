// SqlFile.cpp

#include "SqlFile.h"
#include <system_error>
#include "PDebug.h"
#include <QFile>

SqlFile::SqlFile(QString fn) {
  QFile sqlf(fn);
  ASSERT(sqlf.open(QFile::ReadOnly));
  QString sql = QString(sqlf.readAll());
  QStringList cmds = sql.split(";");
  for (auto c: cmds) {
    c.replace(QRegExp("--[^\\n]*\\n?"), "");
    c.replace(QRegExp("^\\s+"), "");
    c.replace(QRegExp("\\s+$"), "");
    if (!c.isEmpty()) {
      *this << c;
    }
  }
}
