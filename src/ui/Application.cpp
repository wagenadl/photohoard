// Application.cpp

#include "Application.h"
#include "PDebug.h"
#include <QFile>
#include "Version.h"
#include <QFontDatabase>
//#include <QFileSystemWatcher>
#include "Style.h"
#include <QPalette>

Application::Application(int &argc, char **argv):
  QApplication(argc, argv) {
  setApplicationName("Photohoard");
  setApplicationVersion(Version::toString());
  //auto *fsw = new QFileSystemWatcher(this);
  //fsw->addPath(STYLEPATH);
  //connect(fsw, &QFileSystemWatcher::fileChanged,
  //        this, [this]() {
  //          Style::instance()->setLightMode();
  //          });
  QPalette p = palette();
  if (p.text().color().lightness() < p.window().color().lightness())
    Style::instance()->setLightMode();
  else
    Style::instance()->setDarkMode();
  QFontDatabase::addApplicationFont(":Regular.ttf");
  QFontDatabase::addApplicationFont(":Bold.ttf");
  QFontDatabase::addApplicationFont(":Italic.ttf");
  QFontDatabase::addApplicationFont(":BoldItalic.ttf");
}

