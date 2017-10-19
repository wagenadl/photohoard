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
#include "Dialog.h"

AddRootDialog::AddRootDialog(PhotoDB *db, QWidget *parent):
  QDialog(parent), db(db) {
  ui = new Ui_addRootDialog();
  ui->setupUi(this);
  prepCollections();
  Dialog::ensureSize(this);
}

AddRootDialog::~AddRootDialog() {
}

QString AddRootDialog::path() const {
  QString p = ui->location->text();
  if (p.startsWith("~/")) {
    QString home = QString(qgetenv("HOME"));
    p = home + p.mid(1);
  }
  return p;
}

QString AddRootDialog::defaultCollection() const {
  return ui->collection->currentText();
}

int AddRootDialog::exec() {
  prepCollections();
  ui->location->setText(QString(qgetenv("HOME")) + "/Pictures");
  if (Tags(db).collections().isEmpty())
    ui->collection->addItem("Family photos");
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

QString AddRootDialog::reasonableStartingPoint() {
  QDir dir(ui->location->text());
  return dir.exists() ? dir.absolutePath() : QDir::homePath();
}
   

void AddRootDialog::browse() {
  QString dir
    = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                        reasonableStartingPoint(),
                                        QFileDialog::ShowDirsOnly);
  if (!dir.isEmpty())
    ui->location->setText(dir);
}

void AddRootDialog::editExclusion(QModelIndex) {
  QListWidgetItem *item = ui->excluded->currentItem();
  if (!item)
    return;
  QString start = item->text();
  if (start=="(none)")
    start = reasonableStartingPoint();
  QString fn
    = QFileDialog::getExistingDirectory(this,
                                        tr("Select root of tree to exclude"),
                                        start,
                                        QFileDialog::ShowDirsOnly);
  if (fn.isEmpty())
    return;
  item->setText(fn);
}

void AddRootDialog::addExclusion() {
  pDebug() << "AddExclusion";
  QString fn
    = QFileDialog::getExistingDirectory(this,
                                        tr("Select root of tree to exclude"),
                                        path(),
                                        QFileDialog::ShowDirsOnly);
  if (fn.isEmpty())
    return;

  if (ui->excluded->count()==1
      && ui->excluded->item(0)->text() == "none")
    ui->excluded->item(0)->setText(fn);
  else
    ui->excluded->addItem(fn);
}

void AddRootDialog::removeExclusion() {
  pDebug() << "RemoveExclusion";
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

