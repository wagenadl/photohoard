// ImportOtherUserDialog.cpp

#include "ImportOtherUserDialog.h"
#include "SessionDB.h"
#include "ui_ImportOtherUserDialog.h"
#include "Tags.h"
#include "PDebug.h"
#include "ImportJob.h"
#include "SourceInfo.h"

ImportOtherUserDialog::ImportOtherUserDialog(class ImportJob *job,
                                             QStringList collections,
                                             QWidget *parent):
  QWidget(parent), job(job) {
  ui = new Ui_ImportOtherUserDialog;
  ui->setupUi(this);
  if (job->sourceInfo().isTemporaryLike())
    ui->refer->hide();
  what = ui->what->text();
  multi1 = ui->multi1->text();
  multi2 = ui->multi2->text();
  refer = ui->refer->text();
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

ImportOtherUserDialog::~ImportOtherUserDialog() {
}

QString ImportOtherUserDialog::destination() const {
  return ui->destination->text();
}

QString ImportOtherUserDialog::collection() const {
  return ui->collection->currentText();
}

void ImportOtherUserDialog::changeCollection(QString coll) {
  job->setCollection(coll);
  job->setAutoDestination();
  ui->destination->setText(job->destination());
}

void ImportOtherUserDialog::updateCounts(int ntotal, int nmov) {
  int nimg = ntotal - nmov;
  if (nimg==1) 
    ui->what->setText(what.arg("one").arg(""));
  else
    ui->what->setText(what.arg(nimg).arg("s"));
}

void ImportOtherUserDialog::browseDestination() {
  COMPLAIN("NYI");
}
  
bool ImportOtherUserDialog::incorporateInstead() const {
  return ui->refer->isChecked();
}

