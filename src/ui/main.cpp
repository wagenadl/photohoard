// main.cpp

#include "BasicCache.h"
#include <QTime>
#include <QDebug>
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
    qDebug() << "Could not configure serialized database access";
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
      qDebug() << "Unknown command line argument: " << args[0];
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
      qDebug() << "Creating database at " << dbfn;
      PhotoDB::create(dbfn);
    }
    if (!QDir(cachefn).exists()) {
      qDebug() << "Creating cache at " << cachefn;
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
    QObject::connect(scan, SIGNAL(cacheablePreview(quint64, QImage)),
                     ac, SLOT(cachePreview(quint64, QImage)));
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
    qDebug() << "App returned " << res;
    qDebug() << "Stopping scanner";
    scan->stopAndWait(1000);
    qDebug() << "Deleting scanner";
    delete scan;
    qDebug() << "Done";
    qDebug() << "Deleting autocache";
    delete ac;
    qDebug() << "Done";
    qDebug() << "Deleting exporter";
    expo->stop();
    delete expo;
    qDebug() << "Done";

    return res;
    
  } catch (QSqlQuery const &q) {
    qDebug() << "Main caught " << q.lastError();
    qDebug() << "  " << q.lastQuery();
  } catch (...) {
    qDebug() << "Main caught unknown";
  }
}
