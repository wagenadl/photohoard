// main.cpp

#include "BasicCache.h"
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
#include "CMSTransform.h"
#include "SessionDB.h"

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

  CMSProfile rgb(CMSProfile::srgbProfile());
  if (icc=="")
    CMS::monitorProfile = CMSProfile::displayProfile();
  else
    CMS::monitorProfile = CMSProfile(icc);
  CMS::monitorTransform = CMSTransform(CMSProfile::srgbProfile(),
                                       CMS::monitorProfile);

  if (newdb==QFile(dbfn).exists()) {
    if (newdb) 
      pDebug() << "Database not found at " << dbfn;
    else
      pDebug() << "Database already exists at " << dbfn;
    return 2;
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

  MainWindow *mw = new MainWindow(&db, scan, ac, expo);

  QDesktopWidget *dw = app.desktop();
  mw->resize(dw->width()*8/10, dw->height()*8/10);
  mw->move(dw->width()/10, dw->height()/10);
  mw->show();

  mw->scrollToCurrent();

  if (scan)
    scan->start(); // doing this here ensures that the mainwindow can open 1st

  int res = app.exec();
  if (scan) {
    scan->stopAndWait(1000);
    delete scan;
  }
  delete ac;
  expo->stop();
  delete expo;

  PurgeCache::purge(db, cachefn);
    
  db.close();

  return res;
}
