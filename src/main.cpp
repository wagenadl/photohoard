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

int main(int argc, char **argv) {

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

  QString dbfn = "/tmp/photodb.db";
  QString cachefn = "/tmp/photodb.cache";
  QString picroot = "/home/wagenaar/Pictures";
  try {
    Application app(argc, argv);
    if (!QFile(dbfn).exists()) {
      PhotoDB::create(dbfn);
    }
    if (!QDir(cachefn).exists())
      BasicCache::create(cachefn);

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
