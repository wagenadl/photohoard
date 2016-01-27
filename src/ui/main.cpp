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

namespace CMS {
  CMSProfile monitorProfile;
  CMSTransform monitorTransform;
}

void usage() {
  fprintf(stderr, "Usage: photohoard -icc profile -db databasedir\n");
  exit(1);
}

int main(int argc, char **argv) {

  QString dbdir = "/home/wagenaar/.local/photohoard";
  QString icc;
  
  QStringList args;
  for (int i=1; i<argc; i++)
    args << argv[i];
  while (!args.isEmpty()) {
    QString kwd = args.takeFirst();
    if (kwd=="-db") {
      Q_ASSERT(!args.isEmpty());
      dbdir = args.takeFirst();
    } else if (kwd=="-icc") {
      Q_ASSERT(!args.isEmpty());
      icc = args.takeFirst();
    } else {
      usage();
    }
  }

  QString dbfn = dbdir + "/photodb.db";
  QString cachefn = dbdir + "/photodb.cache";

  Application app(argc, argv);

  CMSProfile rgb(CMSProfile::srgbProfile());
  if (icc=="")
    CMS::monitorProfile = CMSProfile::displayProfile();
  else
    CMS::monitorProfile = CMSProfile(icc);
  CMS::monitorTransform = CMSTransform(CMSProfile::srgbProfile(),
                                       CMS::monitorProfile);
  
  if (!QFile(dbfn).exists()) {
    pDebug() << "Creating database at " << dbfn;
    PhotoDB::create(dbfn);
  }

  PhotoDB db;
  db.open(dbfn);
    
  if (!QDir(cachefn).exists()) {
    pDebug() << "Creating cache at " << cachefn;
    BasicCache::create(cachefn);
  }

  AutoCache *ac = new AutoCache(&db, cachefn);

  Scanner *scan = new Scanner(&db);
  QObject::connect(scan, SIGNAL(updated(QSet<quint64>)),
                   ac, SLOT(recache(QSet<quint64>)));
  QObject::connect(scan, SIGNAL(cacheablePreview(quint64, Image16)),
                   ac, SLOT(cachePreview(quint64, Image16)));

  Exporter *expo = new Exporter(&db, 0);
  expo->start();

  MainWindow *mw = new MainWindow(&db, scan, ac, expo);
  QDesktopWidget *dw = app.desktop();
  mw->resize(dw->width()*8/10, dw->height()*8/10);
  mw->move(dw->width()/10, dw->height()/10);
  mw->show();

  mw->scrollToCurrent();
    
  scan->start(); // doing this here ensures that the mainwindow can open 1st

  int res = app.exec();
  scan->stopAndWait(1000);
  delete scan;
  delete ac;
  expo->stop();
  delete expo;

  PurgeCache::purge(db, cachefn);
    
  db.close();

  return res;
}
