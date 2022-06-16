// main.cpp

#include "Settings.h"
#include "ScreenResolution.h"
#include <QTime>
#include "PDebug.h"
#include "Application.h"
#include <QLabel>
#include "NikonLenses.h"
#include <QDesktopWidget>
#include "CMS.h"
#include "CMSTransform.h"
#include "CMS.h"
#include <thread>
#include <iostream>
#include "Session.h"

void myMsgHandler(QtMsgType typ, const QMessageLogContext &/*ctxt*/,
                  const QString &msg) {
  std::cerr<< msg.toUtf8().data() << "\n";
  if (typ==3) {
    std::cerr << "this is serious\n";
  }
}

void usage() {
  fprintf(stderr, "Usage: photohoard -icc profile -ro -new -db database\n");
  exit(1);
}

int main(int argc, char **argv) {
  Settings settings;
  QString dbfn;
  QString icc;
  bool newdb = false;
  bool readonly = false;
  
  QStringList args;
  for (int i=1; i<argc; i++)
    args << argv[i];
  while (!args.isEmpty()) {
    QString kwd = args.takeFirst();
    if (kwd=="-db") {
      Q_ASSERT(!args.isEmpty());
      dbfn = args.takeFirst();
    } else if (kwd=="-icc") {
      Q_ASSERT(!args.isEmpty());
      icc = args.takeFirst();
    } else if (kwd=="-new") {
      newdb = true;
    } else if (kwd=="-ro") {
      readonly = true;
    } else {
      usage();
    }
  }

  Application app(argc, argv);
  app.setFont(ScreenResolution::defaultFont());
  
  CMSProfile rgb(CMSProfile::srgbProfile());
  if (icc=="")
    CMS::monitorProfile = CMSProfile::displayProfile();
  else
    CMS::monitorProfile = CMSProfile(icc);
  CMS::monitorTransform = CMSTransform(CMSProfile::srgbProfile(),
                                       CMS::monitorProfile);

  Session *session = new Session(dbfn, newdb, readonly);
  int res = 2;
  if (session->isActive())
    res = app.exec();
  return res;
}
