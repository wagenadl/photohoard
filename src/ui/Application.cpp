// Application.cpp

#include "Application.h"
#include "PDebug.h"
#include <QFile>
#include "Version.h"

Application::Application(int &argc, char **argv): QApplication(argc, argv) {
  setApplicationName("Photohoard");
  setApplicationVersion(Version::toString());
  QFile f(":/style.css");
  ASSERT(f.open(QFile::ReadOnly));
  setStyleSheet(QString::fromUtf8(f.readAll()));
}

