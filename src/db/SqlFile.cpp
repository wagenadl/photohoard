// SqlFile.cpp

#include "SqlFile.h"
#include <system_error>
#include "PDebug.h"
#include <QFile>

SqlFile::SqlFile(QString fn) {
  QFile sqlf(fn);
  if (!sqlf.open(QFile::ReadOnly)) {
    pDebug() << "SqlFile: Could not open SQL code: " << fn;
    throw std::system_error(std::make_error_code(std::errc::no_such_file_or_directory));
  }
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
