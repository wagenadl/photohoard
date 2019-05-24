// main.cpp

#include "BasicCache.h"
#include "ScreenResolution.h"
#include <QTime>
#include "PDebug.h"
#include "Application.h"
#include <QLabel>
#include "NikonLenses.h"
#include "Exif.h"
#include "Scanner.h"
#include "PhotoDB.h"
#include "PurgeCache.h"
#include "Exporter.h"
#include "AutoCache.h"
#include "MainWindow.h"
#include "ExifReport.h"
#include <sqlite3.h>
#include <QDesktopWidget>
#include "CMS.h"
#include "CMSTransform.h"
#include "CMS.h"
#include "SessionDB.h"
#include "ErrorDialog.h"
#include <thread>

void usage() {
  fprintf(stderr, "Usage: photohoard -icc profile -ro -new -db database\n");
  exit(1);
}

int main(int argc, char **argv) {
  SessionDB::ensureBaseDirExists();
  QString dbfn = SessionDB::photohoardBaseDir() + "/photodb.db";
  QString icc;
  bool newdb = false;
  bool readonly = false;
  bool customdb = false;
  
  QStringList args;
  for (int i=1; i<argc; i++)
    args << argv[i];
  while (!args.isEmpty()) {
    QString kwd = args.takeFirst();
    if (kwd=="-db") {
      Q_ASSERT(!args.isEmpty());
      dbfn = args.takeFirst();
      customdb = true;
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
  //pDebug() << "Application constructed";
  ScreenResolution sr;
  std::thread sr_thread([&sr]() { sr.dpi(); });
  /* Experimentally, calculating the screen reso in a separate thread
     saves 50 ms startup time on hirudo. */

  CMSProfile rgb(CMSProfile::srgbProfile());
  if (icc=="")
    CMS::monitorProfile = CMSProfile::displayProfile();
  else
    CMS::monitorProfile = CMSProfile(icc);
  CMS::monitorTransform = CMSTransform(CMSProfile::srgbProfile(),
                                       CMS::monitorProfile);

  bool olddb = QFile(dbfn).exists();
  if (olddb && newdb) {
      ErrorDialog::fatal("A database already exists at " + dbfn
	      + ". Cannot create a new one.");
      return 2;
  }
  if (!olddb && !newdb) {
    if (customdb) {
      ErrorDialog::fatal("No database found at " + dbfn
                     + ". You may create a new one using “photohoard -new”.");
      return 2;
    } else {
      newdb = true;
    }
  }
  
  if (newdb) {
    pDebug() << "Creating database at " << dbfn;
    PhotoDB::create(dbfn);
  }

  if (!SessionDB::sessionExists(dbfn)) {
    pDebug() << "Creating session for " << dbfn;
    SessionDB::createSession(dbfn);
  }

  SessionDB db;
  //  db.enableDebug();
  db.open(dbfn, readonly);

  QString cachefn = db.cacheFilename();
    
  if (!QDir(cachefn).exists()) {
    pDebug() << "Creating cache at " << cachefn;
    BasicCache::create(cachefn);
  }

  AutoCache *ac = new AutoCache(&db, cachefn);

  Scanner *scan = 0;
  if (!db.isReadOnly()) {
    scan = new Scanner(&db);
    QObject::connect(scan, SIGNAL(updated(QSet<quint64>)),
		     ac, SLOT(recache(QSet<quint64>)));
    QObject::connect(scan, SIGNAL(cacheablePreview(quint64, Image16)),
		     ac, SLOT(cachePreview(quint64, Image16)));
  }

  Exporter *expo = new Exporter(&db, 0);
  expo->start();

  if (scan)
    scan->start(); // doing this here ensures that the mainwindow can open 1st

  app.setFont(ScreenResolution::defaultFont());
  
  // START APPLICATION
  MainWindow *mw = new MainWindow(&db, scan, ac, expo);
  mw->show();
  mw->scrollToCurrent();

  int res = app.exec();

  // END APPLICATION

  if (scan) {
    scan->stopAndWait(1000);
    delete scan;
  }
  delete ac;
  expo->stop();
  delete expo;

  PurgeCache::purge(db, cachefn);
    
  db.close();

  sr_thread.join();
  return res;
}
