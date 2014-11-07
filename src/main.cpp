// main.cpp

#include "BasicCache.h"
#include <QTime>
#include <QDebug>
#include <QApplication>
#include <QLabel>
#include "NikonLenses.h"
#include "Exif.h"
#include "PhotoScanner.h"
#include "FolderScanner.h"

int main(int argc, char **argv) {

  QSet<QString> cmds;
  for (int i=1; i<argc; i++)
    cmds << argv[i];
  
  BasicCache *bc = 0;
  if (QDir("/").exists("/tmp/phfoo"))
    bc = new BasicCache("/tmp/phfoo");
  else
    bc = BasicCache::create("/tmp/phfoo");

  QStringList fns;
  fns << "/home/wagenaar/cincinnati-houses/jora/2nd visit/IMG_4841.JPG";
  fns << "/home/wagenaar/PicsTest/2014-09-21/DSC_1701_1.JPG";
  fns << "/home/wagenaar/PicsTest/2014-09-21/DSC_1752.JPG";
  if (cmds.contains("fill")) {
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
    if (cmds.contains("show")) {
      QApplication app(argc, argv);
      QString fn = fns.last();
      Exif exif(fn);
      QList<QSize> sizes(exif.previewSizes());
      QImage img(exif.previewImage(sizes.last()));
      QLabel foo;
      foo.setPixmap(QPixmap::fromImage(img));
      foo.show();
      app.exec();
      return 0;
    }
  }   

  if (cmds.contains("retrieve")) {
    for (auto fn: fns) {
      QTime t; t.start();
      QImage img(bc->get(qHash(fn), bc->bestSize(qHash(fn), 1024)));
      qDebug() << "Retrieved " << t.elapsed() << ": " << img.size();
    }
  }

  if (cmds.contains("show")) {
    QApplication app(argc, argv);
    QString fn = fns.first();
    QImage img(bc->get(qHash(fn), bc->bestSize(qHash(fn), 1024)));
    QLabel foo;
    foo.setPixmap(QPixmap::fromImage(img));
    foo.show();
    app.exec();
  }

  if (cmds.contains("scan")) {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("/tmp/photodb.db");
    if (!db.open()) {
      qDebug() << "Could not open db";
      return 1;
    }
    QApplication app(argc, argv);
    PhotoScanner pscan(db);
    pscan.start();
    FolderScanner fscan(db, &pscan);
    fscan.start();
    fscan.add("/home/wagenaar/PicsTest");
    QLabel foo("Scanning");
    foo.show();
    app.exec();
  }
  
  if (cmds.contains("lens")) {
    NikonLenses lenses;
    quint64 id = 0x7f402d5c2c348406;
    QString desc = lenses[id];
    qDebug() << id << ": " << desc;
  }
  return 0;
}

  

