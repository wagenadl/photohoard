// ExportDialog.cpp

#include "ExportDialog.h"
#include "ui_ExportDialog.h"
#include "PhotoDB.h"
#include "PDebug.h"
#include <QFileDialog>
#include <QMessageBox>
#include "Exporter.h"

ExportDialog::ExportDialog(bool now, QWidget *parent):
  QDialog(parent) {
  ui = new Ui_exportDialog();
  ui->setupUi(this);
  setup(ExportSettings());
  if (now)
    ui->bOk->hide();
  else
    ui->bExport->hide();
  do_export = false;
}

ExportDialog::~ExportDialog() {
  delete ui;
}

QDialog::DialogCode ExportDialog::exec() {
  while (true) {
    DialogCode res = DialogCode(QDialog::exec());
    if (!res) 
      return res;

    QString dir = ui->destination->text();
    QFileInfo fi(dir);
    if (fi.isDir() && fi.isWritable()) {
      return res;
    }

    QMessageBox::warning(0, "photohoard", 
                         QString::fromUtf8("Folder “")
                         + dir + QString::fromUtf8("” is not a folder"
                                                   " or is not writable."));
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
    QFileInfo fi(dir);
    if (fi.isDir() && fi.isWritable()) {
      return;
    }

    QMessageBox::warning(0, "photohoard", 
                         QString::fromUtf8("Folder “")
                         + dir + QString::fromUtf8("” is not a folder"
                                                   " or is not writable."));
  }
}

void ExportDialog::reset() {
  QString dst = ui->destination->text();
  ExportSettings set;
  set.destination = dst;
  setup(set);
}

void ExportDialog::exportNow() {
  do_export = true;
  accept();
}

void ExportDialog::standalone(class Exporter *exporter, bool now) {
  ExportSettings settings(exporter->settings());
  if (!settings.isValid())
    settings.destination = "/tmp";
  ExportDialog exportdialog(now);
  exportdialog.setup(settings);

  if (exportdialog.exec() == ExportDialog::Accepted) {
    exporter->setup(exportdialog.settings());
    if (exportdialog.do_export)
      exporter->addSelection();
  }
}
