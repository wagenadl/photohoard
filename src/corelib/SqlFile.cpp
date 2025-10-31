// SqlFile.cpp

#include "SqlFile.h"
#include <system_error>
#include "PDebug.h"
#include <QFile>
#include <QRegularExpression>

SqlFile::SqlFile(QString fn) {
  QFile sqlf(fn);
  ASSERT(sqlf.open(QFile::ReadOnly));
  QString sql = QString(sqlf.readAll());
  QStringList cmds = sql.split(";");
  for (auto c: cmds) {
    c.replace(QRegularExpression("--[^\\n]*\\n?"), "");
    c.replace(QRegularExpression("^\\s+"), "");
    c.replace(QRegularExpression("\\s+$"), "");
    if (!c.isEmpty()) {
      *this << c;
    }
  }
}
