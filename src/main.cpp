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
  try {

    QSet<QString> cmds;
    for (int i=1; i<argc; i++)
      cmds << argv[i];

    bool runapp = false;
    Application app(argc, argv);
  
  
    QStringList fns;
    fns << "/home/wagenaar/cincinnati-houses/jora/2nd visit/IMG_4841.JPG";
    fns << "/home/wagenaar/PicsTest/2014-09-21/DSC_1701_1.JPG";
    fns << "/home/wagenaar/PicsTest/2014-09-21/DSC_1752.JPG";
    if (cmds.contains("bcfill")) { // outdated
      BasicCache *bc = 0;
      if (QDir("/").exists("/tmp/phfoo"))
	bc = new BasicCache("/tmp/phfoo");
      else
	bc = BasicCache::create("/tmp/phfoo");
      for (auto fn: fns) {
	QTime t; t.start();
	QImage img(fn);
	qDebug() << "Loaded " << t.elapsed();
	bc->add(qHash(fn), img);
	qDebug() << "Complete " << t.elapsed();
      }
    }

    if (cmds.contains("exif")) {
      for (auto fn: fns) {
	qDebug() << fn;
	Exif exif(fn);
	qDebug() << "  " << exif.width() << "x" << exif.height();
	qDebug() << "  " << exif.camera();
	qDebug() << "  " << exif.lens();
	qDebug() << "  " << exif.dateTime();
	qDebug() << "  t = " << exif.exposureTime_s() << " s";
	qDebug() << "  f = 1/" << exif.fNumber();
	qDebug() << "  ISO " << exif.iso();
	qDebug() << "  F = " << exif.focalLength_mm() << " mm";
	qDebug() << "  d = " << exif.focusDistance_m() << " m";
	qDebug() << "  Previews: " << exif.previewSizes().size();
	qDebug() << "";
      }
    }   
    if (cmds.contains("exifshow")) {
      QString fn = fns.last();
      Exif exif(fn);
      QList<QSize> sizes(exif.previewSizes());
      QImage img(exif.previewImage(sizes.last()));
      QLabel foo;
      foo.setPixmap(QPixmap::fromImage(img));
      foo.show();
      runapp = true;
    }

    if (cmds.contains("retrieve")) { // outdated
      BasicCache *bc = new BasicCache("/tmp/phfoo");
      for (auto fn: fns) {
	QTime t; t.start();
	QImage img(bc->get(qHash(fn), bc->bestSize(qHash(fn), 1024)));
	qDebug() << "Retrieved " << t.elapsed() << ": " << img.size();
      }
    }

    if (cmds.contains("bcshow")) { // outdated
      BasicCache *bc = new BasicCache("/tmp/phfoo");
      QString fn = fns.first();
      QImage img(bc->get(qHash(fn), bc->bestSize(qHash(fn), 1024)));
      QLabel foo;
      foo.setPixmap(QPixmap::fromImage(img));
      foo.show();
      runapp = true;
    }

    if (cmds.contains("create")) {
      PhotoDB::create("/tmp/photodb.db");
      BasicCache::create("/tmp/photodb.cache");
    }
    PhotoDB db("/tmp/photodb.db");
    Scanner *scan = 0;
    AutoCache *ac = 0;
    ExceptionReporter *excrep = new ExceptionReporter();
    
    if (cmds.contains("autocache") || cmds.contains("view")) {
      ac = new AutoCache(db, "/tmp/photodb.cache");
      QObject::connect(ac, SIGNAL(exception(QString)),
		       excrep, SLOT(report(QString)));
      runapp = true;
    }
    if (cmds.contains("scan")) {
      scan = new Scanner(db);
      if (ac && scan) {
	QObject::connect(scan, SIGNAL(updated(QSet<quint64>)),
			 ac, SLOT(recache(QSet<quint64>)));
        QObject::connect(scan, SIGNAL(cacheablePreview(quint64, QImage)),
                         ac, SLOT(cachePreview(quint64, QImage)));
      }
      scan->start();
      scan->addTree("/home/wagenaar/Pictures");
      QObject::connect(scan, SIGNAL(exception(QString)),
	      excrep, SLOT(report(QString)));
      runapp = true;
    }
    if (cmds.contains("view")) {
      LightTable *view = new LightTable(db);
      view->show();
      if (ac) {
	QObject::connect(view, SIGNAL(needImage(quint64, QSize)),
			 ac, SLOT(request(quint64, QSize)));
	QObject::connect(ac, SIGNAL(available(quint64, QSize, QImage)),
			 view, SLOT(updateImage(quint64, QSize, QImage)));
      }
      if (scan) {
        QObject::connect(scan, SIGNAL(updated(QSet<quint64>)),
                         view, SLOT(rescan()));
      }
      runapp = true;
    }
    if (cmds.contains("lens")) {
      NikonLenses lenses;
      quint64 id = 0x7f402d5c2c348406;
      QString desc = lenses[id];
      qDebug() << id << ": " << desc;
    }

    if (runapp)
      return app.exec();
    else
      return 0;

  } catch (QSqlQuery const &q) {
    qDebug() << "Main caught " << q.lastError();
    qDebug() << "  " << q.lastQuery();
  } catch (...) {
    qDebug() << "Main caught unknown";
  }
}

  

