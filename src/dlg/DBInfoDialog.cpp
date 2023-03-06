// DBInfoDialog.cpp

#include "DBInfoDialog.h"
#include "ui_DBInfoDialog.h"
#include "SessionDB.h"
#include <QFileInfo>
#include <QDirIterator>
#include <QDebug>
#include "RootsList.h"
#include "Session.h"
#include "FileLocations.h"

DBInfoDialog::DBInfoDialog(SessionDB *sdb, QWidget *parent):
  QDialog(parent), sdb(sdb) {
  ui = new Ui_DBInfoDialog();
  ui->setupUi(this);
  locationlabel = ui->txtcount->text();
  setup();
}

DBInfoDialog::~DBInfoDialog() {
}

quint64 DBInfoDialog::dirSize(QString root) {
  quint64 size = 0;
  for (QDirIterator it(root, QDir::Files); it.hasNext(); )
    size += QFileInfo(it.next()).size();
  for (QDirIterator it(root, QDir::Dirs | QDir::NoDotAndDotDot);
       it.hasNext(); )
    size += dirSize(it.next());
  return size;
}

QString DBInfoDialog::niceCount(quint64 count) {
  return QString("%L1").arg(count);
}

QString DBInfoDialog::niceSize(quint64 bytesize) {
  quint64 kB = 1000;
  quint64 MB = 1000*kB;
  quint64 GB = 1000*MB;
  quint64 TB = 1000*GB;
  if (bytesize >= 9.9*TB)
    return QString("%1 TB").arg(bytesize/TB);
  else if (bytesize >= TB)
    return QString("%1 TB").arg(1.0*bytesize/TB, 0, 'f', 1);
  else if (bytesize >= 9.9*GB)
    return QString("%1 GB").arg(bytesize/GB);
  else if (bytesize >= GB)
    return QString("%1 GB").arg(1.0*bytesize/GB, 0, 'f', 1);
  else if (bytesize >= 9.9*MB)
    return QString("%1 MB").arg(bytesize/MB);
  else if (bytesize >= MB)
    return QString("%1 MB").arg(1.0*bytesize/MB, 0, 'f', 1);
  else if (bytesize >= 9.9*kB)
    return QString("%1 kB").arg(bytesize/kB);
  else if (bytesize >= kB)
    return QString("%1 kB").arg(1.0*bytesize/kB, 0, 'f', 1);
  else
    return QString("%1 B").arg(bytesize);
}

void DBInfoDialog::setup() {
  QString dbfn = sdb->photoDBFilename();
  QFileInfo dbinfo(dbfn);
  ui->txtdbname->setText(dbinfo.absoluteFilePath());
  ui->txtsession->setText(sdb->sessionFilename());
  ui->txtcache->setText(sdb->cacheDirname());
  ui->dbsize->setText(niceSize(dbinfo.size()));
  ui->cachesize->setText(niceSize(dirSize(sdb->cacheDirname())));
  ui->txtcount->setText(locationlabel
                        .arg(niceCount(sdb->countPhotos()))
                        .arg(niceCount(sdb->rootFolders().size())));
  QStringList roots(sdb->rootFolders());
  ui->locations->setText(roots.join("\n"));
}

