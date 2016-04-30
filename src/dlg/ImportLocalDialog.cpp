// ImportLocalDialog.cpp

#include "ImportLocalDialog.h"
#include "SessionDB.h"
#include "ui_ImportLocalDialog.h"
#include "Tags.h"
#include "PDebug.h"
#include "ImportJob.h"
#include "SourceInfo.h"

ImportLocalDialog::ImportLocalDialog(class ImportJob *job,
                                             QStringList collections,
                                             QWidget *parent):
  QWidget(parent), job(job) {
  ui = new Ui_ImportLocalDialog;
  ui->setupUi(this);
  what = ui->what->text();
  ui->source->setText(ui->source->text() + ": "
                      + job->sourceInfo().commonRoot());
  connect(ui->ok, SIGNAL(clicked()), this, SIGNAL(accepted()));
  connect(ui->cancel, SIGNAL(clicked()), this, SIGNAL(canceled()));

  connect(job, SIGNAL(countsUpdated(int,int)), SLOT(updateCounts(int,int)));

  ui->collection->clear();
  for (auto s: collections)
    ui->collection->addItem(s);
  QString coll = job->collection();  
  int idx = ui->collection->findText(coll);
  if (idx>=0)
    ui->collection->setCurrentIndex(idx);

  ui->destination->setText(job->destination());
}

ImportLocalDialog::~ImportLocalDialog() {
}

QString ImportLocalDialog::destination() const {
  return ui->destination->text();
}

QString ImportLocalDialog::collection() const {
  return ui->collection->currentText();
}

void ImportLocalDialog::changeCollection(QString coll) {
  job->setCollection(coll);
  job->setAutoDestination();
  ui->destination->setText(job->destination());
}

void ImportLocalDialog::updateCounts(int ntotal, int nmov) {
  int nimg = ntotal - nmov;
  if (nimg==1) 
    ui->what->setText(what.arg("one").arg(""));
  else
    ui->what->setText(what.arg(nimg).arg("s"));
}

void ImportLocalDialog::browseDestination() {
  COMPLAIN("NYI");
}

QString ImportLocalDialog::destination() const {
  return ui->destination->text();
}
  
QString ImportLocalDialogcollection() const {
  return ui->collection->currentText();
}

bool ImportLocalDialog::importInstead() const {
  return ui->import->isChecked();
}

