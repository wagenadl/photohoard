// ExportDialog.cpp

#include "ExportDialog.h"
#include "ui_ExportDialog.h"

#include "PhotoDB.h"


ExportDialog::ExportDialog(QWidget *parent): QDialog(parent) {
  ui = new Ui_exportDialog();
  ui->setupUi(this);
  setup(ExportSettings());
  QPushButton *okb = ui->buttonBox->button(QDialogButtonBox::Ok);
  Q_ASSERT(okb);
  okb->setDefault(true);
}

ExportDialog::~ExportDialog() {
  delete ui;
}

QDialog::DialogCode ExportDialog::exec() {
  ExportSettings before = settings();
  DialogCode res = DialogCode(QDialog::exec());
  if (res != Accepted)
    setup(before);
  return res;
}

void ExportDialog::setup(ExportSettings const &s) {
  ui->format->setCurrentIndex(int(s.fileFormat));
  
  switch (s.resolutionMode) {
  case ExportSettings::ResolutionMode::Full:
    ui->rFull->setChecked(true);
    break;
  case ExportSettings::ResolutionMode::LimitWidth:
    ui->rWidth->setChecked(true);
    break;
  case ExportSettings::ResolutionMode::LimitHeight:
    ui->rHeight->setChecked(true);
    break;
  case ExportSettings::ResolutionMode::LimitMaxDim:
    ui->rMaxDim->setChecked(true);
    break;
  case ExportSettings::ResolutionMode::Scale:
    ui->rMaxDim->setChecked(true);
    break;
  }

  ui->maxdim->setValue(s.maxdim);
  ui->scale->setValue(s.scalePercent);
  ui->quality->setValue(s.jpegQuality);
  ui->scheme->setCurrentIndex(int(s.namingScheme));
  ui->destination->setText(s.destination);
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

  s.maxdim = ui->maxdim->value();
  s.scalePercent = ui->scale->value();
  s.namingScheme = ExportSettings::NamingScheme(ui->scheme->currentIndex());
  s.destination = ui->destination->text();
  return s;
}

