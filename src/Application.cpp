// Application.cpp

#include "Application.h"
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>

Application::Application(int &argc, char **argv): QApplication(argc, argv) {
}

bool Application::notify(QObject *receiver, QEvent *e) {
  bool b = false;
  try {
    b = QApplication::notify(receiver, e);
  } catch (QSqlQuery &q) {
    qDebug() << "Uncaught exception: SqlError: " << q.lastError().text()
	     << " from " << q.lastQuery();
    quit();
  } catch (...) {
    qDebug() << "Uncaught exception";
    quit();
  }
  return b;
}
