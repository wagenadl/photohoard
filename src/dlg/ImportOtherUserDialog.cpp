// ImportOtherUserDialog.cpp

#include "ImportOtherUserDialog.h"
#include "SessionDB.h"
#include "ui_ImportOtherUserDialog.h"
#include "Tags.h"
#include "PDebug.h"
#include "ImportJob.h"
#include "SourceInfo.h"
#include <QKeyEvent>
#include <QFileDialog>

ImportOtherUserDialog::ImportOtherUserDialog(class ImportJob *job,
                                             QStringList collections,
                                             QWidget *parent):
  QWidget(parent), job(job) {
  ui = new Ui_ImportOtherUserDialog;
  ui->setupUi(this);
  if (job->sourceInfo().isTemporaryLike()
      || !job->sourceInfo().isOnlyFolders())
    ui->refer->hide();
  ui->movieContainer->hide();
  what = ui->what->text();
  multi1 = ui->multi1->text();
  multi2 = ui->multi2->text();
  refer = ui->refer->text();
  copymov = ui->copyMovies->text();
  ui->source->setText(job->sourceInfo().simplifiedRoot());
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

  ui->destination->setText(SourceInfo::simplified(job->destination()));

  resize(minimumSizeHint());
}

ImportOtherUserDialog::~ImportOtherUserDialog() {
}

QString ImportOtherUserDialog::destination() const {
  return SourceInfo::reconstructed(ui->destination->text());
}

QString ImportOtherUserDialog::movieDestination() const {
  return ui->movieDestination->text();
}

bool ImportOtherUserDialog::hasMovieDestination() const {
  return ui->copyMovies->isChecked();
}


QString ImportOtherUserDialog::collection() const {
  return ui->collection->currentText();
}

void ImportOtherUserDialog::changeCollection(QString coll) {
  //  job->setCollection(coll);
  //  job->setAutoDestination();
  //  ui->destination->setText(job->destination());
}

void ImportOtherUserDialog::updateCounts(int ntotal, int nmov) {
  int nimg = ntotal - nmov;
  if (nimg==1) {
    ui->what->setText(what.arg("one").arg(""));
    ui->multi1->setText(multi1.arg(""));
    ui->multi2->setText(multi2.arg("The o").arg(""));
    ui->refer->setText(refer.arg("a").arg("").arg(""));
  } else {
    ui->what->setText(what.arg(nimg).arg("s"));
    ui->multi1->setText(multi1.arg("s"));
    ui->multi2->setText(multi2.arg("O").arg("s"));
    ui->refer->setText(refer.arg("").arg("s").arg("s"));
  }
  if (nmov==1)
    ui->copyMovies->setText(copymov.arg("one").arg(""));
  else
    ui->copyMovies->setText(copymov.arg(nmov).arg("s"));
  if (nmov>0)
    ui->movieContainer->show();

}

void ImportOtherUserDialog::browseDestination() {
  QString dir = QFileDialog::getExistingDirectory(this, tr("Select destination"),
                                                  ui->destination->text(),
                                                  QFileDialog::ShowDirsOnly);
  if (!dir.isEmpty())
    ui->destination->setText(dir);
}
  
bool ImportOtherUserDialog::incorporateInstead() const {
  return ui->refer->isChecked();
}

void ImportOtherUserDialog::keyPressEvent(QKeyEvent *e) {
  if (e->key()==Qt::Key_Escape)
    emit canceled();
  else
    QWidget::keyPressEvent(e);
}
