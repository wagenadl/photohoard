// DatabaseDialog.cpp

#include "DatabaseDialog.h"
#include "ui_DatabaseDialog.h"
#include "SessionDB.h"
#include <QFileInfo>
#include <QDirIterator>
#include <QDebug>
#include "RootsList.h"
#include "Session.h"
#include "CreateDatabaseDialog.h"
#include <QFileDialog>
#include "FileLocations.h"

DatabaseDialog::DatabaseDialog(SessionDB *sdb, QWidget *parent):
  QDialog(parent), sdb(sdb) {
  ui = new Ui_databaseDialog();
  ui->setupUi(this);
  ui->brename->hide();
  ui->bmovedb->hide();
  ui->bmovesession->hide();
  ui->bmovecache->hide();
}

DatabaseDialog::~DatabaseDialog() {
}

quint64 DatabaseDialog::dirSize(QString root) {
  quint64 size = 0;
  for (QDirIterator it(root, QDir::Files); it.hasNext(); )
    size += QFileInfo(it.next()).size();
  for (QDirIterator it(root, QDir::Dirs | QDir::NoDotAndDotDot);
       it.hasNext(); )
    size += dirSize(it.next());
  return size;
}

QString DatabaseDialog::niceCount(quint64 count) {
  return QString("%L1").arg(count);
}

QString DatabaseDialog::niceSize(quint64 bytesize) {
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

void DatabaseDialog::setup() {
  QString dbfn = sdb->photoDBFilename();
  QFileInfo dbinfo(dbfn);
  ui->txtdbname->setText(dbinfo.fileName());
  ui->txtlocation->setText(dbinfo.canonicalPath());
  ui->txtsession->setText(sdb->sessionFilename());
  ui->txtcache->setText(sdb->cacheDirname());
  ui->dbsize->setText(niceSize(dbinfo.size()));
  ui->cachesize->setText(niceSize(dirSize(sdb->cacheDirname())));
  ui->txtcount->setText(QString("Database contains %1 photos in %2 locations.")
                        .arg(niceCount(sdb->countPhotos()))
                        .arg(niceCount(sdb->rootFolders().size())));
  resize(sizeHint());

  QStringList recent = Session::recentDatabases();
  int idx = recent.indexOf(dbfn);
  if (idx>=0)
    recent.removeAt(idx);
  if (recent.isEmpty())
    recent.append("(none)");
  ui->listrecent->clear();
  ui->listrecent->addItems(recent);
}

void DatabaseDialog::moveDB() {
  qDebug() << "DatabaseDialog: moveDB NYI";
}

void DatabaseDialog::moveSession() {
  qDebug() << "DatabaseDialog: moveSession NYI";
}

void DatabaseDialog::moveCache() {
  qDebug() << "DatabaseDialog: moveCache NYI";
}

void DatabaseDialog::renameDB() {
  qDebug() << "DatabaseDialog: renameDB NYI";
}

void DatabaseDialog::createNew() {
  QDialog *dlg = new CreateDatabaseDialog();
  dlg->setAttribute(Qt::WA_DeleteOnClose);
  dlg->show();
  close();
}

void DatabaseDialog::openOther() {
  close();
  QString fn = QFileDialog::getOpenFileName(0, "Open other database...",
                                            FileLocations::dataRoot(),
                                            "*.db");
  if (fn=="")
    return;
  
  new Session(fn);
}

void DatabaseDialog::openRecent(QModelIndex idx) {
  close();
  QString fn = idx.data().toString();
  if (fn=="(none)" || fn=="")
    return;
  new Session(fn);
}

void DatabaseDialog::showRoots() {
  RootsList *dlg = new RootsList(sdb);
  dlg->setAttribute(Qt::WA_DeleteOnClose);
  dlg->show();
}

void DatabaseDialog::splitDatabase() {
  qDebug() << "DatabaseDialog: splitDatabase";
}

void DatabaseDialog::incorporateDatabase() {
  qDebug() << "DatabaseDialog: incorporateDatabase";
}
