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

const QString noneLabel = "(none)";

AddRootDialog::AddRootDialog(PhotoDB *db, QWidget *parent):
  QDialog(parent), db(db) {
  ui = new Ui_addRootDialog();
  ui->setupUi(this);
  ui->excluded->addItem(noneLabel);
  prepCollections();
  Dialog::ensureSize(this);
  connect(ui->excluded, &QListWidget::currentItemChanged,
          this, [this]() {
            QListWidgetItem *item = ui->excluded->currentItem();
            ui->removeExcluded->setEnabled(item && item->text()!=noneLabel);
          });
}

AddRootDialog::~AddRootDialog() {
}

QString AddRootDialog::path() const {
  QString p = ui->location->text();
  if (p.startsWith("~/")) 
    return QDir::homePath() + p.mid(1);
  else
    return QFileInfo(p).absoluteFilePath();
}

QString AddRootDialog::defaultCollection() const {
  QString col = ui->collection->currentText();
  return col==noneLabel ? QString() : col;
}

int AddRootDialog::exec() {
  prepCollections();
  ui->location->setText(QDir::homePath() + "/Pictures");
  DialogCode c = DialogCode(QDialog::exec());
  return c;
}

void AddRootDialog::prepCollections() {
  ui->collection->clear();
  ui->collection->addItem(noneLabel);
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

void AddRootDialog::addExclusion() {
  pDebug() << "AddExclusion";
  QString fn
    = QFileDialog::getExistingDirectory(this,
                                        tr("Select root of tree to exclude"),
                                        path(),
                                        QFileDialog::ShowDirsOnly);
  if (fn.isEmpty())
    return;

  fn = QFileInfo(fn).absoluteFilePath();

  if (ui->excluded->count()==1
      && ui->excluded->item(0)->text() == noneLabel)
    ui->excluded->item(0)->setText(fn);
  else
    ui->excluded->addItem(fn);
}

void AddRootDialog::removeExclusion() {
  QListWidgetItem *item = ui->excluded->currentItem();       
  if (item)
    delete item;
  if (ui->excluded->count()==0)
    ui->excluded->addItem(noneLabel);
}

QStringList AddRootDialog::exclusions() const {
  QStringList ee;
  for (int n=0; n<ui->excluded->count(); n++) {
    QString fn = ui->excluded->item(n)->text();
    if (fn.isEmpty() || fn==noneLabel)
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

  if (!QDir(path()).exists()) {
    if (interactive)
      QMessageBox::warning(0, "Photohoard",
			   "The specified path could not be found. Please"
			   " check your typing.",
			   QMessageBox::Ok);
    return false;
  }
  
  //if (defaultCollection().isEmpty()) {
  //  if (interactive)
  //    QMessageBox::warning(0, "Photohoard", QString::fromUtf8(
  //          "Every image in Photohoard should be part of at"
  //          " least one “collection.” Please select a"
  //          " default collection for your new folder tree."
  //          " You may also type the name of a new collection."),
  //          QMessageBox::Ok);
  //  return false;
  //}

  if (db->findFolder(path())) {
    if (interactive)
      QMessageBox::warning(0, "Photohoard", 
            "The specified folder is already included."
            " Therefore, it cannot be added again.",
	    QMessageBox::Ok);
    return false;
  }

  return true;
}

