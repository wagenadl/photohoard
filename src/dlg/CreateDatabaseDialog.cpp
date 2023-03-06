// CreateDatabaseDialog.cpp

#include "CreateDatabaseDialog.h"
#include "ui_CreateDatabaseDialog.h"
#include "SessionDB.h"
#include "FileLocations.h"
#include "PDebug.h"
#include <QFile>
#include "Session.h"
#include <QMessageBox>
#include <QFileDialog>

CreateDatabaseDialog::CreateDatabaseDialog(QWidget *parent):
  QDialog(parent) {
  ui = new Ui_createDatabaseDialog();
  ui->setupUi(this);
}

CreateDatabaseDialog::~CreateDatabaseDialog() {
}

void CreateDatabaseDialog::showEvent(QShowEvent *e) {
  QDialog::showEvent(e);
  setup();
}

void CreateDatabaseDialog::setup() {
  QString name = FileLocations::dataRoot() + "/Untitled.photohoardDB";
  int n = 0 ;
  while (QFileInfo(name).exists()) {
    n += 1;
    name = (FileLocations::dataRoot() + "/Untitled%1.photohoardDB").arg(n);
  }
    
  ui->dblocation->setText(name);
  ui->cachelocation->setText(FileLocations::cacheRoot());
}

void CreateDatabaseDialog::browseLocation() {
  QString fn = QFileDialog::getSaveFileName(this,
                                            "Database filename…",
                                            FileLocations::dataRoot(),
                                            "*.photohoardDB");
  if (!fn.isEmpty())
    ui->dblocation->setText(fn);
}

void CreateDatabaseDialog::browseCache() {
  QString dir = QFileDialog::getExistingDirectory(this,
                                                  "Cache location…",
                                                  ui->cachelocation->text());
  if (!dir.isEmpty())
    ui->cachelocation->setText(dir);
}

void CreateDatabaseDialog::accept() {
  qDebug() << "accept";
  QString dbfn = ui->dblocation->text();
  QString cachelocation = ui->cachelocation->text();
  if (!dbfn.endsWith(".photohoardDB"))
    dbfn += ".photohoardDB";
  qDebug() << "  dbfn " << dbfn;
  qDebug() << "  cachelocation " << cachelocation;
  if (QFile(dbfn).exists()) {
    QMessageBox::warning(0, "Photohoard", "A database named “" + dbfn
                         + "” already exists."
                         + " Cannot create a new one.");
    return;
  }
  new Session(dbfn, true, false, cachelocation);
  QDialog::accept();
}
