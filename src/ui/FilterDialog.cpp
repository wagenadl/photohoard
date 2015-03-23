// FilterDialog.cpp

#include "FilterDialog.h"
#include "PDebug.h"
#include "ui_FilterDialog.h"

FilterDialog::FilterDialog(QWidget *parent): QDialog(parent) {
  ui = new Ui_FilterDialog();
  ui->setupUi(this);
}

void FilterDialog::recount() {
  pDebug() << "FD::recount";
}

void FilterDialog::setMaker() {
  pDebug() << "FD::setMaker";
}

void FilterDialog::setCamera() {
  pDebug() << "FD::setCamera";
}

void FilterDialog::recolorTags() {
  pDebug() << "FD::recolorTags";
}
