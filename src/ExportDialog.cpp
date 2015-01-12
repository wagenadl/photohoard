// ExportDialog.cpp

#include "ExportDialog.h"

#include "ui_ExportDialog.h"

ExportDialog::Settings::Settings() {
  fileFormat = FileFormat::JPEG;
  resolutionMode = ResolutionMode::Full;
  maxdim = 1000;
  scalePercent = 50;
  jpegQuality = 90;
  namingScheme = NamingScheme::Original;
  destination = "/tmp";
}

ExportDialog::ExportDialog(QWidget *parent): QDialog(parent) {
  ui = new Ui_exportDialog();
  ui->setupUi(this);
  setup(ExportDialog::Settings());
}

ExportDialog::~ExportDialog() {
  delete ui;
}

QDialog::DialogCode ExportDialog::exec() {
  ExportDialog::Settings before = settings();
  DialogCode res = DialogCode(QDialog::exec());
  if (res != Accepted)
    setup(before);
  return res;
}

void ExportDialog::setup(ExportDialog::Settings const &s) {
  ui->format->setCurrentIndex(int(s.fileFormat));
  
  switch (s.resolutionMode) {
  case Settings::ResolutionMode::Full:
    ui->rFull->setChecked(true);
    break;
  case Settings::ResolutionMode::LimitWidth:
    ui->rWidth->setChecked(true);
    break;
  case Settings::ResolutionMode::LimitHeight:
    ui->rHeight->setChecked(true);
    break;
  case Settings::ResolutionMode::LimitMaxDim:
    ui->rMaxDim->setChecked(true);
    break;
  case Settings::ResolutionMode::Scale:
    ui->rMaxDim->setChecked(true);
    break;
  }

  ui->maxdim->setValue(s.maxdim);
  
  ui->scale->setValue(s.scalePercent);

  ui->quality->setValue(s.jpegQuality);

  ui->scheme->setCurrentIndex(int(s.namingScheme));

  ui->destination->setText(s.destination);
}

ExportDialog::Settings ExportDialog::settings() const {
  Settings s;

  s.fileFormat = Settings::FileFormat(ui->format->currentIndex());

  s.resolutionMode
    = ui->rFull->isChecked() ? Settings::ResolutionMode::Full
    : ui->rWidth->isChecked() ? Settings::ResolutionMode::LimitWidth
    : ui->rHeight->isChecked() ? Settings::ResolutionMode::LimitHeight
    : ui->rMaxDim->isChecked() ? Settings::ResolutionMode::LimitMaxDim
    : Settings::ResolutionMode::Scale;

  s.maxdim = ui->maxdim->value();

  s.scalePercent = ui->scale->value();

  s.namingScheme = Settings::NamingScheme(ui->scheme->currentIndex());

  s.destination = ui->destination->text();

  return s;
}
