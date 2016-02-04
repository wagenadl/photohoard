// AllControls.cpp

#include "AllControls.h"
#include "Action.h"
#include "ControlSliders.h"
#include "Cropper.h"

AllControls::AllControls(QWidget *parent): QTabWidget(parent) {
  sliders = new ControlSliders();
  addTab(sliders, QIcon(":/icons/sliders.svg"), "Sliders");
  connect(sliders, SIGNAL(valueChanged(QString, double)),
	  SLOT(valueChange(QString, double)));
  cropper = new Cropper();
  addTab(cropper, "Crop");
}

AllControls::~AllControls() {
}

Adjustments const &AllControls::getAll() const {
  return sliders->getAll();
}  

void AllControls::setAll(Adjustments const &a) {
  sliders->setAll(a);
}

void AllControls::valueChange(QString adjuster, double value) {
  emit valueChanged(adjuster, value);
}

QSize AllControls::sizeHint() const {
  return QTabWidget::sizeHint();
  //  return sliders->sizeHint(); // a bit taller...
}

Actions const &AllControls::actions() {
  return ControlSliders::actions();
}
