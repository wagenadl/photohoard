// LayerDialog.cpp

#include "LayerDialog.h"
#include "PDebug.h"
#include "ui_LayerDialog.h"

LayerDialog::LayerDialog(QWidget *parent): QWidget(parent) {
  ui = new Ui_LayerDialog;
  ui->setupUi(this);
  while (ui->table->rowCount() > 0)
    ui->table->removeRow(ui->table->rowCount()-1);
}

LayerDialog::~LayerDialog() {
}

void LayerDialog::addGradientLayer() {
}

void LayerDialog::addLayer() {
}

void LayerDialog::deleteLayer() {
}

void LayerDialog::raiseLayer() {
}

void LayerDialog::lowerLayer() {
}

void LayerDialog::showHideLayer() {
}

void LayerDialog::showHideMask() {
}

void LayerDialog::newSelection() {
  pDebug() << "new selection";
}

void LayerDialog::respondToClick(int r, int c) {
  pDebug() << "click" << r << c;
}