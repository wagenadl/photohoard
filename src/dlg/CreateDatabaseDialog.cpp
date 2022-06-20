// CreateDatabaseDialog.cpp

#include "CreateDatabaseDialog.h"
#include "ui_CreateDatabaseDialog.h"
#include "SessionDB.h"
#include "FileLocations.h"
#include "PDebug.h"
#include <QFile>
#include "Session.h"
#include <QMessageBox>

CreateDatabaseDialog::CreateDatabaseDialog(QWidget *parent):
  QDialog(parent) {
  ui = new Ui_createDatabaseDialog();
  ui->setupUi(this);
  ui->browsecachelocation->hide();
  ui->browsedblocation->hide();
  ui->dblocation->setReadOnly(true);
  ui->dblocation->setFocusPolicy(Qt::NoFocus);
  ui->cachelocation->setReadOnly(true);
  ui->cachelocation->setFocusPolicy(Qt::NoFocus);
}

CreateDatabaseDialog::~CreateDatabaseDialog() {
}

void CreateDatabaseDialog::showEvent(QShowEvent *e) {
  QDialog::showEvent(e);
  setup();
  ui->dbname->setFocus();
  ui->dbname->setSelection(0, ui->dbname->text().size());
}

void CreateDatabaseDialog::setup() {
  ui->dblocation->setText(FileLocations::dataRoot());
  ui->cachelocation->setText(FileLocations::cacheRoot());
  ui->dbname->setText("untitled");
}

void CreateDatabaseDialog::browseLocation() {
  qDebug() << "CreateDatabaseDialog::browseLocation NYI";
}

void CreateDatabaseDialog::browseCache() {
  qDebug() << "CreateDatabaseDialog::browseCache NYI";
}

void CreateDatabaseDialog::accept() {
  qDebug() << "accept";
  QString dbname = ui->dbname->text();
  QString dblocation = ui->dblocation->text();
  QString cachelocation = ui->cachelocation->text();
  if (!dbname.endsWith(".db"))
    dbname += ".db";
  qDebug() << "  dbname " << dbname;
  qDebug() << "  dblocation " << dblocation;
  qDebug() << "  cachelocation " << cachelocation;
  QString dbfn = dblocation + "/" + dbname;
  if (QFile(dbfn).exists()) {
    QMessageBox::warning(0, "Photohoard", "A database named “" + dbname
                         + "“ already exists in “" + dblocation
                         + "”. Cannot create a new one.");
    return;
  }
  new Session(dbfn, true);
  QDialog::accept();
}
