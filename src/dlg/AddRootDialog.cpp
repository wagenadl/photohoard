// AddRootDialog.cpp

#include "AddRootDialog.h"
#include "Tags.h"
#include "ui_AddRootDialog.h"
#include <QKeyEvent>
#include <QFileDialog>
#include "PDebug.h"
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>

AddRootDialog::AddRootDialog(PhotoDB *db, QWidget *parent):
  QDialog(parent), db(db) {
  ui = new Ui_addRootDialog();
  ui->setupUi(this);
  prepCollections();
}

AddRootDialog::~AddRootDialog() {
}

QString AddRootDialog::path() const {
  return ui->location->text();
}

QString AddRootDialog::defaultCollection() const {
  return ui->collection->currentText();
}

QDialog::DialogCode AddRootDialog::exec() {
  prepCollections();
  DialogCode c = DialogCode(QDialog::exec());
  return c;
}

void AddRootDialog::prepCollections() {
  ui->collection->clear();
  for (QString c: Tags(db).collections())
    ui->collection->addItem(c);
}

void AddRootDialog::keyPressEvent(QKeyEvent *e) {
  switch (e->key()) {
  case Qt::Key_Return:
    if (e->modifiers() & Qt::ControlModifier) 
      accept();
    break;
  default:
    QDialog::keyPressEvent(e);
    break;
  }    
}

void AddRootDialog::browse() {
  QString dir
    = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                        QDir::homePath(),
                                        QFileDialog::ShowDirsOnly);
  if (!dir.isEmpty())
    ui->location->setText(dir);
}

void AddRootDialog::editExclusion(QModelIndex) {
  QListWidgetItem *item = ui->excluded->currentItem();
  if (!item)
    return;
  if (item->text()=="(none)")
    return;
  QString fn
    = QFileDialog::getExistingDirectory(this,
                                        "Select root of tree to exclude",
                                        item->text());
  if (fn.isEmpty())
    return;
  item->setText(fn);
}

void AddRootDialog::addExclusion() {
  QString fn
    = QFileDialog::getExistingDirectory(this,
                                        "Select root of tree to exclude",
                                        path());
  if (fn.isEmpty())
    return;

  if (ui->excluded->count()==1
      && ui->excluded->item(0)->text() == "none")
    ui->excluded->item(0)->setText(fn);
  else
    ui->excluded->addItem(fn);
}

void AddRootDialog::removeExclusion() {
  int n = ui->excluded->currentRow();
  if (n>=0 && n<ui->excluded->count())
    ui->excluded->removeItemWidget(ui->excluded->item(n));
  if (ui->excluded->count()==0)
    ui->excluded->addItem("(none)");
}

QStringList AddRootDialog::exclusions() const {
  QStringList ee;
  for (int n=0; n<ui->excluded->count(); n++) {
    QString fn = ui->excluded->item(n)->text();
    if (fn.isEmpty() || fn=="(none)")
      continue;
    ee << fn;
  }
  return ee;
}
    
bool AddRootDialog::validate(bool interactive) const {
  if (path().isEmpty()) {
    if (interactive)
      QMessageBox::warning(0, "Photohoard",
			   "To add a new folder tree,"
			   " you must specify its path.",
			   QMessageBox::Ok);
    return false;
  }

  QDir root(path());
  if (!root.exists()) {
    if (interactive)
      QMessageBox::warning(0, "Photohoard",
			   "The specified path could not be found. Please"
			   " check your typing.",
			   QMessageBox::Ok);
    return false;
  }
  
  if (defaultCollection().isEmpty()) {
    if (interactive)
      QMessageBox::warning(0, "Photohoard", QString::fromUtf8(
	    "Every image in Photohoard should be part of at"
	    " least one “collection.” Please select a"
	    " default collection for your new folder tree."
	    " You may also type the name of a new collection."),
	    QMessageBox::Ok);
    return false;
  }

  return true;
}
