// main.cpp

#include "Settings.h"
#include <QTime>
#include "FileLocations.h"
#include "Application.h"
#include <QLabel>
#include "CMS.h"
#include "CMSTransform.h"
#include "CMS.h"
#include <QFileInfo>
#include <iostream>
#include "Session.h"
#include <QCommandLineOption>
#include <QCommandLineParser>
#include "corelib/DateTime.h"
#include <QSplashScreen>
#include "Style.h"
#include <QDeadlineTimer>
#include <QTimer>

void myMsgHandler(QtMsgType typ, const QMessageLogContext &/*ctxt*/,
                  const QString &msg) {
  std::cerr << msg.toUtf8().data() << "\n";
  if (typ==3) {
    std::cerr << "this is serious\n";
  }
}

int main(int argc, char **argv) {
  qRegisterMetaType<DateTime>();
  Settings settings;
  Application app(argc, argv);
  qDebug() << "app";

  QDeadlineTimer t0;
  t0.setRemainingTime(4000);
  QPixmap pixmap(Style::instance()->isDark() ? ":/banner-dark.png" : ":/banner.png");
  pixmap.setDevicePixelRatio(2);
  QSplashScreen splash(pixmap);
  splash.setWindowFlag(Qt::WindowStaysOnTopHint, true);
  Image16::setHidpiFactor(splash.devicePixelRatio());
  splash.setFixedSize(pixmap.size() / 2);
  splash.show();
  splash.showMessage("");
  app.processEvents();
  qDebug() << "splash" << splash.devicePixelRatio() << pixmap.size() << pixmap.devicePixelRatio();
  

  QCommandLineOption cli_db("db", "Specify database to use",
                            "file.photohoard");
  QCommandLineOption cli_icc("icc", "Specify ICC color profile",
                             "file.icc");
  QCommandLineOption cli_new("new", "Create new database");
  QCommandLineOption cli_ro("ro", "Open database read-only");

  QCommandLineParser cli;
  cli.setApplicationDescription("\n"
    "Photohoard is a collection manager for digital photographs.\n"
    "More information is at https://github.com/wagenadl/photohoard.");
  cli.addHelpOption();                    
  cli.addVersionOption();
  cli.addOption(cli_db);
  cli.addOption(cli_icc);
  cli.addOption(cli_new);
  cli.addOption(cli_ro);
  cli.process(app);
  QStringList args = cli.positionalArguments();

  QString dbfn = cli.value("db");
  QString icc = cli.value("icc");
  bool newdb = cli.isSet("new");
  bool readonly = cli.isSet("ro");

  FileLocations::ensureDataRoot();
  app.processEvents();

  CMSProfile rgb(CMSProfile::srgbProfile());
  if (icc != "") {
    if (QFileInfo(icc).exists()) {
      CMS::monitorProfile = CMSProfile(icc);
    } else {
      qDebug() << "ICC profile" << icc << "not found";
      return 1;
    }
  } else {
    CMS::monitorProfile = CMSProfile::displayProfile();
  }
  CMS::monitorTransform = CMSTransform(CMSProfile::srgbProfile(),
                                       CMS::monitorProfile);

  app.processEvents();

  Session *session = new Session(dbfn, newdb, readonly);

  //  QTimer::singleShot(t0.remainingTimeAsDuration(), [&splash, &session]() {
  splash.finish(session->mainWindow());
  //});
  
//  splash.finish(session->mainWindow());
  if (session->isActive())
    return app.exec();
  else
    return 2;
}
