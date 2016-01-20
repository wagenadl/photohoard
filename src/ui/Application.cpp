// Application.cpp

#include "Application.h"
#include "PDebug.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>

Application::Application(int &argc, char **argv): QApplication(argc, argv) {
  QFile f(":/style.css");
  ASSERT(f.open(QFile::ReadOnly));
  setStyleSheet(QString::fromUtf8(f.readAll()));
}

