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
#include "Exporter.h"
#include "AutoCache.h"
#include "ExceptionReporter.h"
#include "MainWindow.h"
#include "ExifReport.h"
#include <sqlite3.h>
#include <QDesktopWidget>
#include "CMSTransform.h"

namespace CMS {
  CMSProfile monitorProfile;
  CMSTransform monitorTransform;
}

int main(int argc, char **argv) {
  /*
  if (sqlite3_config(SQLITE_CONFIG_SERIALIZED) != SQLITE_OK) {
    pDebug() << "Could not configure serialized database access";
    return 1;
  }
  */

  QString dbdir = "/home/wagenaar/.local/photohoard";
  QString picroot = "/home/wagenaar/Pictures";
  QString icc;
  
  QStringList args;
  for (int i=1; i<argc; i++)
    args << argv[i];
  while (!args.isEmpty()) {
    QString kwd = args.takeFirst();
    if (kwd=="-exif") {
      for (auto s: args) {
        exifreport(s);
      }
      return 0;
    } else if (kwd=="-root") {
      Q_ASSERT(!args.isEmpty());
      picroot = args.takeFirst();
    } else if (kwd=="-db") {
      Q_ASSERT(!args.isEmpty());
      dbdir = args.takeFirst();
    } else if (kwd=="-icc") {
      Q_ASSERT(!args.isEmpty());
      icc = args.takeFirst();
    } else {
      pDebug() << "Unknown command line argument: " << args[0];
      return 1;
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
  
  try {
    if (!QFile(dbfn).exists()) {
      pDebug() << "Creating database at " << dbfn;
      PhotoDB::create(dbfn);
    }
    if (!QDir(cachefn).exists()) {
      pDebug() << "Creating cache at " << cachefn;
      delete BasicCache::create(cachefn);
    }

    ExceptionReporter *excrep = new ExceptionReporter();

    PhotoDB db(dbfn);

    AutoCache *ac = new AutoCache(db, cachefn);
    QObject::connect(ac, SIGNAL(exception(QString)),
                     excrep, SLOT(report(QString)));

    Scanner *scan = new Scanner(db);
    QObject::connect(scan, SIGNAL(updated(QSet<quint64>)),
                     ac, SLOT(recache(QSet<quint64>)));
    QObject::connect(scan, SIGNAL(cacheablePreview(quint64, Image16)),
                     ac, SLOT(cachePreview(quint64, Image16)));
    QObject::connect(scan, SIGNAL(exception(QString)),
                     excrep, SLOT(report(QString)));
    scan->start();

    Exporter *expo = new Exporter(db, 0);
    expo->start();

    scan->addTree(picroot); // this should not always happen

    MainWindow *mw = new MainWindow(db, scan, ac, expo);
    QDesktopWidget *dw = app.desktop();
    mw->resize(dw->width()*8/10, dw->height()*8/10);
    mw->move(dw->width()/10, dw->height()/10);
    mw->show();

    mw->scrollToCurrent();
    
    int res = app.exec();
    pDebug() << "App returned " << res;
    pDebug() << "Stopping scanner";
    scan->stopAndWait(1000);
    pDebug() << "Deleting scanner";
    delete scan;
    pDebug() << "Done";
    pDebug() << "Deleting autocache";
    delete ac;
    pDebug() << "Done";
    pDebug() << "Deleting exporter";
    expo->stop();
    delete expo;
    pDebug() << "Done";

    return res;
    
  } catch (QSqlQuery const &q) {
    pDebug() << "Main caught " << q.lastError();
    pDebug() << "  " << q.lastQuery();
  } catch (...) {
    pDebug() << "Main caught unknown";
  }
}
