// AllControls.cpp

#include "AllControls.h"
#include "Action.h"
#include "ControlSliders.h"
#include "Cropper.h"

AllControls::AllControls(QWidget *parent): QTabWidget(parent) {
  sliders = new ControlSliders();
  addTab(sliders, QIcon(":/icons/sliders.svg"), "Sliders");
  connect(sliders, SIGNAL(valueChanged(QString, double)),
	  SLOT(changeFromSliders(QString, double)));
  cropper = new Cropper();
  addTab(cropper, QIcon(":/icons/crop.svg"), "Crop");
  connect(cropper, SIGNAL(rectangleChanged(QRect, QSize)),
	  SLOT(changeFromCropper(QRect, QSize)));
}

AllControls::~AllControls() {
}

Adjustments const &AllControls::getAll() const {
  return sliders->getAll();
}  

void AllControls::setAll(Adjustments const &a, PSize origsize) {
  sliders->setAll(a);
  cropper->setAll(a, origsize);
}

void AllControls::changeFromSliders(QString adjuster, double value) {
  emit valueChanged(adjuster, value);
  cropper->setValue(adjuster, value);
}

void AllControls::changeFromCropper(QRect croprect, QSize osize) {
  Adjustments const &adj0 = getAll();
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

  /* Perhaps a "valuesChanged" signal would be useful. It would take a
     QMap<QString, double> as argument. */
  if (adj.cropl!=adj0.cropl)
    emit valueChanged("cropl", adj.cropl);
  if (adj.cropr!=adj0.cropr)
    emit valueChanged("cropr", adj.cropr);
  if (adj.cropt!=adj0.cropt)
    emit valueChanged("cropt", adj.cropt);
  if (adj.cropb!=adj0.cropb)
    emit valueChanged("cropb", adj.cropb);
}

QSize AllControls::sizeHint() const {
  return QTabWidget::sizeHint();
  //  return sliders->sizeHint(); // a bit taller...
}

Actions const &AllControls::actions() {
  return ControlSliders::actions();
}
