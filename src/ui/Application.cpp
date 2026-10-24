// Application.cpp

#include "Application.h"
#include "PDebug.h"
#include <QFile>
#include "Version.h"
#include <QFontDatabase>
//#include <QFileSystemWatcher>
#include "Style.h"
#include <QPalette>
#include <QStyleHints>

#if defined(Q_OS_UNIX) && !defined(Q_OS_DARWIN) && !defined(Q_OS_ANDROID)
#include <QDBusInterface>
#include <QDBusConnection>

static Qt::ColorScheme portal_scheme() {
  QDBusInterface iface("org.freedesktop.portal.Desktop",
                       "/org/freedesktop/portal/desktop",
                       "org.freedesktop.portal.Settings",
                       QDBusConnection::sessionBus());
  auto reply = iface.call("ReadOne", "org.freedesktop.appearance", "color-scheme");
  if (reply.type() == QDBusMessage::ReplyMessage) {
    auto arg = reply.arguments()[0];
    if (arg == 1)
      return Qt::ColorScheme::Dark;
    else if (arg == 2)
      return Qt::ColorScheme::Light;
  }
  return Qt::ColorScheme::Unknown;
}
#else
static Qt::ColorScheme portal_scheme() {
  return Qt::ColorScheme::Unknown;
}
#endif


Application::Application(int &argc, char **argv):
  QApplication(argc, argv) {
  setApplicationName("Photohoard");
  setApplicationVersion(Version::toString());
  // auto *fsw = new QFileSystemWatcher(this);
  // fsw->addPath(STYLEPATH);
  // connect(fsw, &QFileSystemWatcher::fileChanged,
  //         this, [this]() {
  //           Style::instance()->setLightMode();
  //           });
  Qt::ColorScheme scheme = styleHints()->colorScheme();
  if (scheme == Qt::ColorScheme::Unknown)
    scheme = portal_scheme();
  if (scheme == Qt::ColorScheme::Unknown) {
    QPalette p = palette();
    if (p.text().color().lightness() < p.window().color().lightness())
      scheme = Qt::ColorScheme::Dark;
    else
      scheme = Qt::ColorScheme::Light;
  }
  if (scheme == Qt::ColorScheme::Light)
    Style::instance()->setLightMode();
  else
    Style::instance()->setDarkMode();
  QFontDatabase::addApplicationFont(":Regular.ttf");
  QFontDatabase::addApplicationFont(":Bold.ttf");
  QFontDatabase::addApplicationFont(":Italic.ttf");
  QFontDatabase::addApplicationFont(":BoldItalic.ttf");
}

