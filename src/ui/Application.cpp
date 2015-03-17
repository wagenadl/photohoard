// Application.cpp

#include "Application.h"
#include "PDebug.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include "NoResult.h"

Application::Application(int &argc, char **argv): QApplication(argc, argv) {
  QFile f(":/style.css");
  if (!f.open(QFile::ReadOnly))
    throw QString("Cannot open style");
  setStyleSheet(QString::fromUtf8(f.readAll()));
}

bool Application::notify(QObject *receiver, QEvent *e) {
  bool b = false;
  try {
    b = QApplication::notify(receiver, e);
  } catch (QSqlQuery &q) {
    pDebug() << "Uncaught exception: SqlError: " << q.lastError().text()
	     << " from " << q.lastQuery();
    quit();
  } catch (NoResult &n) {
    pDebug() << "Uncaught exception: NoResult" << n.msg << n.n;
  } catch (...) {
    pDebug() << "Uncaught exception: unknown";
    quit();
  }
  return b;
}
