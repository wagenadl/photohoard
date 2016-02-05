// Cropper.cpp

#include "Cropper.h"
#include "ui_Cropper.h"

Cropper::Cropper(QWidget *parent): QWidget(parent) {
  ui = new Ui_Cropper();
  ui->setupUi(this);
}

void Cropper::setRect(QRect r, QSize s) {
  origSize = s;
  setRect(r);
}

void Cropper::setRect(QRect r) {
  ui->tgt_free->setChecked(true);
  reflectRect();
}

Cropper::RatioMode Cropper::ratioMode() {
  if (ui->ratio_auto->isChecked())
    return RatioMode::Auto;
  if (ui->ratio_landscape->isChecked())
    return RatioMode::Landscape;
  if (ui->ratio_Portrait->isChecked())
    return RatioMode::Portrait;
  return RatioMode::Auto; // hmmm.
}

void Cropper::setRatioMode(Cropper::RatioMode r) {
  switch (r) {
  case RatioMode::Auto:
    ui->ratio_auto->setChecked(true);
    break;
  case RatioMode::Portrait:
    ui->ratio_portrait->setChecked(true);
    break;
  case RatioMode::Landscaope:
    ui->ratio_landscaope->setChecked(true);
    break;
  }
}
