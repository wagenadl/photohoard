// ImportExternalDialog.cpp

#include "ImportExternalDialog.h"
#include "SessionDB.h"
#include "ui_ImportExternalDialog.h"
#include "Tags.h"
#include "PDebug.h"
#include "ImportJob.h"
#include "SourceInfo.h"
#include <QKeyEvent>
#include <QFileDialog>

ImportExternalDialog::ImportExternalDialog(ImportJob *job,
                                           QStringList collections,
                                           QWidget *parent):
  QWidget(parent), job(job) {
  ui = new Ui_ImportExternalDialog;
  ui->setupUi(this);

  if (job->sourceInfo().isTemporaryLike()) {
    ui->disposition->removeItem(1);
    ui->source->setText("Temporary location");
    bkremoved = true;
  } else {
    bkremoved = false;
  }
  changeDisposition();
  
  what = ui->what->text();
  movieWhat = ui->copyMovies->text();
  ui->source->setText(ui->source->text() + ": "
                      + job->sourceInfo().simplifiedRoot());

  QString home = QString(qgetenv("HOME"));
  ui->movieDestination->setText(ImportJob::autoDest(home
                                                    + "/Pictures/movies"));
  ui->movieContainer->hide();

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

  switch (job->sourceDisposition()) {
  case CopyIn::SourceDisposition::Leave:
    ui->disposition->setCurrentIndex(0);
    break;
  case CopyIn::SourceDisposition::Backup:
    ui->disposition->setCurrentIndex(bkremoved ? 0 : 1);
    break;
  case CopyIn::SourceDisposition::Delete:
    ui->disposition->setCurrentIndex(bkremoved ? 1 : 2);
    break;
  }
  resize(minimumSizeHint());
}

ImportExternalDialog::~ImportExternalDialog() {
}

QString ImportExternalDialog::destination() const {
  return SourceInfo::reconstructed(ui->destination->text());
}

QString ImportExternalDialog::movieDestination() const {
  return ui->movieDestination->text();
}

bool ImportExternalDialog::hasMovieDestination() const {
  return ui->copyMovies->isChecked();
}

QString ImportExternalDialog::collection() const {
  return ui->collection->currentText();
}

CopyIn::SourceDisposition ImportExternalDialog::sourceDisposition() const {
  switch (ui->disposition->currentIndex()) {
  case 0:
    return CopyIn::SourceDisposition::Leave;
  case 1:
    return bkremoved
      ? CopyIn::SourceDisposition::Delete
      : CopyIn::SourceDisposition::Backup;
  case 2:
    return CopyIn::SourceDisposition::Delete;
  default:
    CRASH("Unknown disposition");
    return CopyIn::SourceDisposition::Leave;
  }
}

void ImportExternalDialog::changeCollection(QString coll) {
  job->setCollection(coll);
  job->setAutoDestination();
  ui->destination->setText(job->destination());
}  

void ImportExternalDialog::updateCounts(int ntotal, int nmov) {
  int nimg = ntotal - nmov;
  if (nimg==1) 
    ui->what->setText(what.arg("one").arg(""));
  else
    ui->what->setText(what.arg(nimg).arg("s"));
  if (nmov==1)
    ui->copyMovies->setText(movieWhat.arg("one").arg(""));
  else
    ui->copyMovies->setText(movieWhat.arg(nmov).arg("s"));
  if (nmov)
    ui->movieContainer->show();
}

void ImportExternalDialog::browseDestination() {
  QString dir = QFileDialog::getExistingDirectory(this,
                                                  tr("Select destination"),
                                                  ui->destination->text(),
                                                  QFileDialog::ShowDirsOnly);
  if (!dir.isEmpty())
    ui->destination->setText(dir);
}

void ImportExternalDialog::browseMovieDestination() {
  QString dir = QFileDialog::getExistingDirectory(this,
                                                  tr("Select destination"),
                                                  ui->movieDestination->text(),
                                                  QFileDialog::ShowDirsOnly);
  if (!dir.isEmpty())
    ui->movieDestination->setText(dir);
}

void ImportExternalDialog::keyPressEvent(QKeyEvent *e) {
  if (e->key()==Qt::Key_Escape)
    emit canceled();
  else
    QWidget::keyPressEvent(e);
}

void ImportExternalDialog::changeDisposition() {
  if (sourceDisposition()==CopyIn::SourceDisposition::Backup) {
    ui->backupContainer->show();
    ui->backupPath->setText(QString::fromUtf8("⇒ ") + job->backupPath());
    // set backup path
  } else {
    ui->backupContainer->hide();
  }
}
