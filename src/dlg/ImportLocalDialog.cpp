// ImportLocalDialog.cpp

#include "ImportLocalDialog.h"
#include "SessionDB.h"
#include "ui_ImportLocalDialog.h"
#include "Tags.h"
#include "PDebug.h"
#include "ImportJob.h"
#include "SourceInfo.h"
#include <QKeyEvent>
#include <QFileDialog>

ImportLocalDialog::ImportLocalDialog(class ImportJob *job,
                                             QStringList collections,
                                             QWidget *parent):
  QWidget(parent), job(job) {
  ui = new Ui_ImportLocalDialog;
  ui->setupUi(this);

  if (job->sourceInfo().isTemporaryLike()) {
    ui->disposition->removeItem(1);
    ui->source->setText("Temporary location");
    bkremoved = true;
  } else {
    bkremoved = false;
  }
  changeDisposition();
  
  what = ui->copy->text();
  movieWhat = ui->copyMovies->text();

  if (job->sourceInfo().isSingleFolder()) {
    ui->source->setText(ui->source->text() + ": "
			+ job->sourceInfo().simplifiedRoot());
    what = what.arg("a ").arg("");
  } else {
    ui->source->setText(QString("%1 local folders")
			.arg(job->sourceInfo().sources().count()));
    what = what.arg("").arg("s");
  }
  ui->copy->setText(what.arg("").arg("s"));

  QString home = QString(qgetenv("HOME"));
  ui->movieDestination->setText(ImportJob::autoDest(home + "/Pictures/movies"));
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

ImportLocalDialog::~ImportLocalDialog() {
}

QString ImportLocalDialog::destination() const {
  return SourceInfo::reconstructed(ui->destination->text());
}

QString ImportLocalDialog::movieDestination() const {
  return ui->movieDestination->text();
}

bool ImportLocalDialog::hasMovieDestination() const {
  return ui->copyMovies->isChecked();
}

CopyIn::SourceDisposition ImportLocalDialog::sourceDisposition() const {
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

void ImportLocalDialog::updateCounts(int ntotal, int nmov) {
  int nimg = ntotal - nmov;
  if (nimg==1) 
    ui->copy->setText(what.arg("one").arg(""));
  else
    ui->copy->setText(what.arg(nimg).arg("s"));
  if (nmov==1)
    ui->copyMovies->setText(movieWhat.arg("one").arg(""));
  else
    ui->copyMovies->setText(movieWhat.arg(nmov).arg("s"));
  if (nmov)
    ui->movieContainer->show();
}

QString ImportLocalDialog::collection() const {
  return ui->collection->currentText();
}

void ImportLocalDialog::changeCollection(QString coll) {
  job->setCollection(coll);
  job->setAutoDestination();
  ui->destination->setText(job->destination());
}

void ImportLocalDialog::browseDestination() {
  QString dirname
    = QFileDialog::getExistingDirectory(this, tr("Select destination"),
                                        ui->destination->text(),
                                        QFileDialog::ShowDirsOnly);
  if (!dirname.isEmpty())
    ui->destination->setText(dirname);
}


void ImportLocalDialog::browseMovieDestination() {
  QString dir = QFileDialog::getExistingDirectory(this,
                                                  tr("Select destination"),
                                                  ui->movieDestination->text(),
                                                  QFileDialog::ShowDirsOnly);
  if (!dir.isEmpty())
    ui->movieDestination->setText(dir);
}

bool ImportLocalDialog::importInstead() const {
  return ui->copy->isChecked();
}

void ImportLocalDialog::keyPressEvent(QKeyEvent *e) {
  if (e->key()==Qt::Key_Escape)
    emit canceled();
  else
    QWidget::keyPressEvent(e);
}

void ImportLocalDialog::changeMode(bool b) {
  if (b)
    ui->ok->setText("Import");
  else
    ui->ok->setText("Incorporate");
}

void ImportLocalDialog::changeDisposition() {
  if (sourceDisposition()==CopyIn::SourceDisposition::Backup) {
    ui->backupContainer->show();
    ui->backupPath->setText(QString::fromUtf8("⇒ ") + job->backupPath());
    // set backup path
  } else {
    ui->backupContainer->hide();
  }
}
