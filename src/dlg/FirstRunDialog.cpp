// FirstrunDialog.cpp

#include "FirstRunDialog.h"
#include "ui_FirstRunDialog.h"
#include "SessionDB.h"
#include "FileLocations.h"
#include "PDebug.h"
#include <QFile>
#include "Session.h"
#include <QMessageBox>
#include <QFileDialog>

FirstRunDialog::FirstRunDialog(QWidget *parent): QDialog(parent) {
  ui = new Ui_FirstRunDialog();
  ui->setupUi(this);
  QString fld = FileLocations::defaultImageRoot();
  if (QFileInfo(fld).exists()) 
    ui->folderList->addItem(fld);
   else 
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
  
  connect(ui->folderList, &QListWidget::currentItemChanged,
          this, [this]() {
            QListWidgetItem *item = ui->folderList->currentItem();
            ui->removeButton->setEnabled(item);
          });
  connect(ui->addButton, &QAbstractButton::clicked,
          this, [this]() { add(); });
  connect(ui->removeButton, &QAbstractButton::clicked,
          this, [this]() { drop(); });
}

FirstRunDialog::~FirstRunDialog() {
}

static bool isSubFolder(QString parent, QString child) {
  // identical copy from CreateDatabaseDialog
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

void FirstRunDialog::add() {
  // identical copy of "addFolder" in CreateDatabaseDialog
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

void FirstRunDialog::drop() {
  // identical copy of "removeFolder" in CreateDatabaseDialog
  QListWidgetItem *item = ui->folderList->currentItem();       
  if (!item)
    return;
  delete item;
  if (ui->folderList->count()>0)
    return;
  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}  

QStringList FirstRunDialog::roots() const {
  QStringList res;
  for (int i=0; i<ui->folderList->count(); i++) {
    QListWidgetItem *item = ui->folderList->item(i);
    if (item)
      res << item->text();
  }
  return res;
}

