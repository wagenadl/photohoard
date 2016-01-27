// ExportDialog.cpp

#include "ExportDialog.h"
#include "ui_ExportDialog.h"
#include "PhotoDB.h"
#include "PDebug.h"
#include <QFileDialog>
#include <QMessageBox>

ExportDialog::ExportDialog(PhotoDB *db, QWidget *parent):
  QDialog(parent), db(db) {
  ever_okd = false;
  ui = new Ui_exportDialog();
  ui->setupUi(this);
  setup(ExportSettings(db));
  QPushButton *okb = ui->buttonBox->button(QDialogButtonBox::Ok);
  Q_ASSERT(okb);
  okb->setDefault(true);
}

ExportDialog::~ExportDialog() {
  delete ui;
}

QDialog::DialogCode ExportDialog::exec() {
  ExportSettings before = settings();
  while (true) {
    DialogCode res = DialogCode(QDialog::exec());
    if (res!=Accepted) {
      setup(before);
      return res;
    }

    QString dir = ui->destination->text();
    QDir d(dir);
    if (d.exists()) {
      ever_okd = true;
      return res;
    }

    QMessageBox::warning(0, "photohoard", 
                         QString::fromUtf8("Folder “")
                         + dir + QString::fromUtf8("” does not exist.")
                         + " Please try again.");
  }
}

void ExportDialog::setup(ExportSettings const &s) {
  ui->format->setCurrentIndex(int(s.fileFormat));
  ui->scale->setEnabled(false);
  ui->maxdim->setEnabled(false);
  
  switch (s.resolutionMode) {
  case ExportSettings::ResolutionMode::Full:
    ui->rFull->setChecked(true);
    break;
  case ExportSettings::ResolutionMode::LimitWidth:
    ui->rWidth->setChecked(true);
    ui->maxdim->setEnabled(true);
    break;
  case ExportSettings::ResolutionMode::LimitHeight:
    ui->rHeight->setChecked(true);
    ui->maxdim->setEnabled(true);
    break;
  case ExportSettings::ResolutionMode::LimitMaxDim:
    ui->rMaxDim->setChecked(true);
    ui->maxdim->setEnabled(true);
    break;
  case ExportSettings::ResolutionMode::Scale:
    ui->rMaxDim->setChecked(true);
    ui->scale->setEnabled(true);
    break;
  }

  ui->maxdim->setValue(s.maxdim);
  ui->scale->setValue(s.scalePercent);
  ui->quality->setValue(s.jpegQuality);
  ui->scheme->setCurrentIndex(int(s.namingScheme));
  ui->destination->setText(s.destination);
}

void ExportDialog::setFormat(int) {
  switch (ExportSettings::FileFormat(ui->format->currentIndex())) {
  case ExportSettings::FileFormat::JPEG:
    ui->quality->setEnabled(true);
    break;
  default:
    ui->quality->setEnabled(false);
    break;
  }
}

void ExportDialog::setResolutionMode() {
  ui->scale->setEnabled(ui->rScale->isChecked());
  ui->maxdim->setEnabled(ui->rMaxDim->isChecked()
                         || ui->rWidth->isChecked()
                         || ui->rHeight->isChecked());
}  

ExportSettings ExportDialog::settings() const {
  ExportSettings s;

  s.fileFormat = ExportSettings::FileFormat(ui->format->currentIndex());

  s.resolutionMode
    = ui->rFull->isChecked() ? ExportSettings::ResolutionMode::Full
    : ui->rWidth->isChecked() ? ExportSettings::ResolutionMode::LimitWidth
    : ui->rHeight->isChecked() ? ExportSettings::ResolutionMode::LimitHeight
    : ui->rMaxDim->isChecked() ? ExportSettings::ResolutionMode::LimitMaxDim
    : ExportSettings::ResolutionMode::Scale;
  
  s.jpegQuality = ui->quality->value();
  s.maxdim = ui->maxdim->value();
  s.scalePercent = ui->scale->value();
  s.namingScheme = ExportSettings::NamingScheme(ui->scheme->currentIndex());
  s.destination = ui->destination->text();
  return s;
}


void ExportDialog::browse() {
  QDir d(ui->destination->text());
  QString dir = "";
  while (true) {
    dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                            d.exists() ? d.absolutePath()
                                            : QDir::homePath(),
                                            QFileDialog::ShowDirsOnly);
    if (dir.isEmpty())
      return;
    QDir d(dir);
    if (d.exists()) {
      ui->destination->setText(dir);
      return;
    }
    QMessageBox::warning(0, "photohoard", QString::fromUtf8("Folder “")
                         + dir + QString::fromUtf8("” does not exist.")
                         + " Please try again.");
  }
}

bool ExportDialog::everOKd() const {
  return ever_okd;
}
