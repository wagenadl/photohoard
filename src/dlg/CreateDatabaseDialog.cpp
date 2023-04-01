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
  connect(ui->folderList, &QListWidget::currentItemChanged,
          this, [this]() {
            QListWidgetItem *item = ui->folderList->currentItem();
            ui->removeButton->setEnabled(item);
          });
}

CreateDatabaseDialog::~CreateDatabaseDialog() {
}

void CreateDatabaseDialog::showEvent(QShowEvent *e) {
  QDialog::showEvent(e);
  setup();
}

void CreateDatabaseDialog::setup() {
  QString root = QDir::homePath() + "/Documents";
  QString name = root + "/Untitled.photohoard";
  int n = 0;
  while (QFileInfo(name).exists()) {
    n += 1;
    name = (root + "/Untitled%1.photohoard").arg(n);
  }
    
  ui->dblocation->setText(name);
  ui->cachelocation->setText(FileLocations::cacheRoot());
  ui->collapse->setTarget(ui->advanced);
  ui->collapse->hideTarget();
  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

void CreateDatabaseDialog::browseLocation() {
  QString fn = QFileDialog::getSaveFileName(this,
                                            "Database filename…",
                                            QDir::homePath() + "/Documents",
                                            "*.photohoard");
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

static bool isSubFolder(QString parent, QString child) {
  // identical copy from FirstRunDialog
  QDir par(parent);
  QDir ch(child);
  if (ch==par)
    return true;
  while (!ch.isRoot()) {
    ch.cdUp();
    if (ch==par)
      return true;
  }
  return false;
}

void CreateDatabaseDialog::addFolder() {
  // identical copy of "add" in FirstRunDialog
  QString newfld = QFileDialog::getExistingDirectory(this,
                                                     "Select an image folder…");
  if (newfld.isEmpty()) // nothing selected
    return;
  newfld = QFileInfo(newfld).canonicalFilePath();
  for (int i=0; i<ui->folderList->count(); i++) {
    QListWidgetItem *item = ui->folderList->item(i);
    if (!item)
      continue;
    QString existingfld = item->text();
    if (newfld==existingfld) {
      QMessageBox::warning(this, "Photohoard",
                           "You cannot add the same folder twice.");
      return;
    } else if (isSubFolder(existingfld, newfld)) {
      QMessageBox::warning(this, "Photohoard",
                           "You cannot add a folder that is contained inside"
                           " an already selected folder.");
      return;
    } else if (isSubFolder(newfld, existingfld)) {
      QMessageBox::warning(this, "Photohoard",
                           "You cannot add a folder that contains"
                           "  an already selected folder.");
      return;
    }
  }
  ui->folderList->addItem(newfld);
  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

void CreateDatabaseDialog::removeFolder() {
  // identical copy of "drop" in FirstRunDialog
  QListWidgetItem *item = ui->folderList->currentItem();       
  if (!item)
    return;
  delete item;
  if (ui->folderList->count()>0)
    return;
  auto *but = ui->buttonBox->button(QDialogButtonBox::Ok);
  if (but)
    but->setEnabled(false);
}  


void CreateDatabaseDialog::accept() {
  qDebug() << "accept";
  QString dbfn = ui->dblocation->text();
  QString cachelocation = ui->cachelocation->text();
  if (!dbfn.endsWith(".photohoard"))
    dbfn += ".photohoard";
  qDebug() << "  dbfn " << dbfn;
  qDebug() << "  cachelocation " << cachelocation;
  if (QFile(dbfn).exists()) {
    QMessageBox::warning(0, "Photohoard", "A database named “" + dbfn
                         + "” already exists."
                         + " Cannot create a new one.");
    return;
  }

  QStringList roots;
  for (int i=0; i<ui->folderList->count(); i++) {
    QListWidgetItem *item = ui->folderList->item(i);
    if (item)
      roots << item->text();
  }

  new Session(dbfn, true, false, cachelocation, roots);

  QDialog::accept();
}
