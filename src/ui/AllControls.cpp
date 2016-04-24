// AllControls.cpp

#include "AllControls.h"
#include "Action.h"
#include "ControlSliders.h"
#include "CropControls.h"

AllControls::AllControls(bool ro, QWidget *parent): QTabWidget(parent) {
  sliders = new ControlSliders(ro);
  addTab(sliders, QIcon(":/icons/sliders.svg"), "Sliders");
  connect(sliders, SIGNAL(valuesChanged()),
	  SLOT(changeFromSliders()));
  if (ro) {
    cropper = 0;
  } else {
    cropper = new CropControls();
    addTab(cropper, QIcon(":/icons/crop.svg"), "Crop");
    connect(cropper, SIGNAL(rectangleChanged(QRect, QSize)),
	    SLOT(changeFromCropper(QRect, QSize)));
  }
}

AllControls::~AllControls() {
}

Adjustments const &AllControls::getAll() const {
  return sliders->getAll();
}  

void AllControls::setAll(Adjustments const &a, QSize origsize) {
  sliders->setAll(a);
  if (cropper)
    cropper->setAll(a, origsize);
}

void AllControls::changeFromSliders() {
  emit valuesChanged();
  if (cropper)
    cropper->setAll(sliders->getAll());
}

void AllControls::changeFromCropper(QRect croprect, QSize osize) {
  Adjustments adj0 = getAll();
  Adjustments adj = adj0;
  adj.cropl = croprect.left();
  adj.cropr = osize.width() - croprect.right();
  adj.cropt = croprect.top();
  adj.cropb = osize.height() - croprect.bottom();
  if (adj.cropl!=adj0.cropl
      || adj.cropr!=adj0.cropr
      || adj.cropt!=adj0.cropt
      || adj.cropb!=adj0.cropb)
    sliders->setAll(adj);

  emit valuesChanged();
}

QSize AllControls::sizeHint() const {
  return QTabWidget::sizeHint();
  //  return sliders->sizeHint(); // a bit taller...
}

Actions const &AllControls::actions() {
  return ControlSliders::actions();
}
