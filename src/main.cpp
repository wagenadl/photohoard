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
#include "AutoCache.h"
#include "ExceptionReporter.h"
#include "MainWindow.h"
#include "ExifReport.h"
#include <sqlite3.h>
#include <QDesktopWidget>

int main(int argc, char **argv) {
  /*
  if (sqlite3_config(SQLITE_CONFIG_SERIALIZED) != SQLITE_OK) {
    qDebug() << "Could not configure serialized database access";
    return 1;
  }
  */

  QStringList args;
  for (int i=1; i<argc; i++)
    args << argv[i];
  if (!args.isEmpty()) {
    if (args[0]=="-exif") {
      args.takeFirst();
      for (auto s: args) {
        exifreport(s);
      }
      return 0;
    }
  }

  QString dbfn = "/home/wagenaar/.local/photohoard/photodb.db";
  QString cachefn = "/home/wagenaar/.local/photohoard/photodb.cache";
  QString picroot = "/home/wagenaar/Pictures";
  try {
    Application app(argc, argv);
    if (!QFile(dbfn).exists()) {
      qDebug() << "Creating database at " << dbfn;
      PhotoDB::create(dbfn);
    }
    if (!QDir(cachefn).exists()) {
      qDebug() << "Creating cache at " << cachefn;
      BasicCache::create(cachefn);
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

    scan->start();
    scan->addTree(picroot);
    QObject::connect(scan, SIGNAL(exception(QString)),
                     excrep, SLOT(report(QString)));

    MainWindow *mw = new MainWindow(&db, scan, ac);
    QDesktopWidget *dw = app.desktop();
    mw->resize(dw->width()*8/10, dw->height()*8/10);
    mw->move(dw->width()/10, dw->height()/10);
    mw->show();
    
    int res = app.exec();

    qDebug() << "Stopping scanner";
    scan->stopAndWait(1000);
    qDebug() << "Deleting scanner";
    delete scan;
    qDebug() << "Done";
    qDebug() << "Deleting autocache";
    delete ac;
    qDebug() << "Done";

    return res;
    
  } catch (QSqlQuery const &q) {
    qDebug() << "Main caught " << q.lastError();
    qDebug() << "  " << q.lastQuery();
  } catch (...) {
    qDebug() << "Main caught unknown";
  }
}
