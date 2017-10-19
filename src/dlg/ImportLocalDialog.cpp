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
  QString copy = ui->copy->text();
  if (job->sourceInfo().isSingleFolder()) {
    ui->source->setText(ui->source->text() + ": "
			+ job->sourceInfo().simplifiedRoot());
    copy = copy.arg("a ").arg("");
  } else {
    ui->source->setText(QString("%1 local folders")
			.arg(job->sourceInfo().sources().count()));
    copy = copy.arg("").arg("s");
  }
  ui->copy->setText(copy);

  connect(ui->ok, SIGNAL(clicked()), this, SIGNAL(accepted()));
  connect(ui->cancel, SIGNAL(clicked()), this, SIGNAL(canceled()));

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

ImportLocalDialog::~ImportLocalDialog() {
}

QString ImportLocalDialog::destination() const {
  return SourceInfo::reconstructed(ui->destination->text());
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
