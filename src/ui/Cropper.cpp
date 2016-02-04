// Cropper.cpp

#include "Cropper.h"
#include "ui_Cropper.h"

Cropper::Cropper(QWidget *parent): QWidget(parent) {
  ui = new Ui_Cropper();
  ui->setupUi(this);
}
