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
#include "LightTable.h"


int main(int argc, char **argv) {
  QString dbfn = "/tmp/photodb.db";
  QString cachefn = "/tmp/photodb.cache";
  QString picroot = "/home/wagenaar/Pictures";
  try {
    Application app(argc, argv);
    if (!QFile(dbfn).exists()) {
      PhotoDB::create(dbfn);
    }
    if (!QFile(cachefn).exists())
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

    LightTable *view = new LightTable(db);
    view->show();
    QObject::connect(view, SIGNAL(needImage(quint64, QSize)),
                     ac, SLOT(request(quint64, QSize)));
    QObject::connect(ac, SIGNAL(available(quint64, QSize, QImage)),
                     view, SLOT(updateImage(quint64, QSize, QImage)));
    
    QObject::connect(scan, SIGNAL(updated(QSet<quint64>)),
                     view, SLOT(rescan()));
    
    return app.exec();
  } catch (QSqlQuery const &q) {
    qDebug() << "Main caught " << q.lastError();
    qDebug() << "  " << q.lastQuery();
  } catch (...) {
    qDebug() << "Main caught unknown";
  }
}

  

