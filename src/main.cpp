// main.cpp

#include "BasicCache.h"
#include <QTime>
#include <QDebug>
#include <QApplication>
#include <QLabel>
#include "NikonLenses.h"

int main(int argc, char **argv) {

  QSet<QString> cmds;
  for (int i=1; i<argc; i++)
    cmds << argv[i];
  
  BasicCache *bc = 0;
  if (QDir("/").exists("/tmp/phfoo"))
    bc = new BasicCache("/tmp/phfoo");
  else
    bc = BasicCache::create("/tmp/phfoo");

  QString fns = "090828-Sam-Fireman.jpg 2003-Ellinor-Hoogwater.jpg dw-mugshot.jpg img_0117.jpg IMG_1456.jpg IMG_1468.jpg IMG_1896.jpg IMG_3482.jpg";

  if (cmds.contains("fill")) {
    for (auto fn: fns.split(" ")) {
      QTime t; t.start();
      QImage img("/home/wagenaar/Pictures/" + fn);
      qDebug() << "Loaded " << t.elapsed();
      bc->add(qHash(fn), img);
      qDebug() << "Complete " << t.elapsed();
    }
  }

  if (cmds.contains("retrieve")) {
    for (auto fn: fns.split(" ")) {
      QTime t; t.start();
      QImage img(bc->get(qHash(fn), bc->bestSize(qHash(fn), 1024)));
      qDebug() << "Retrieved " << t.elapsed() << ": " << img.size();
    }
  }

  if (cmds.contains("show")) {
    QApplication app(argc, argv);
    QString fn = fns.split(" ").first();
    QImage img(bc->get(qHash(fn), bc->bestSize(qHash(fn), 1024)));
    QLabel foo;
    foo.setPixmap(QPixmap::fromImage(img));
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

  

