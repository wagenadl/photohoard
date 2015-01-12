// ExportDialog.cpp

#include "ExportDialog.h"

#include "ui_ExportDialog.h"

ExportDialog::ExportDialog(QWidget *parent): QDialog(parent) {
  ui = new Ui_exportDialog();
  ui->setupUi(this);
}

ExportDialog::~ExportDialog() {
  delete ui;
}
