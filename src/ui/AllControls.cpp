// AllControls.cpp

#include "AllControls.h"
#include "Action.h"
#include "ControlSliders.h"
#include "CropControls.h"
#include "LayerDialog.h"
#include "PhotoDB.h"

AllControls::AllControls(PhotoDB *db, QWidget *parent):
  QTabWidget(parent), db(db) {
  vsn = 0;

  bool ro = db->isReadOnly();
  
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

  layers = new LayerDialog();
  addTab(layers, QIcon(":/icons/layers.svg"), "Layers");
}

AllControls::~AllControls() {
}

Adjustments const *AllControls::getAll(quint64 v) const {
  if (v==vsn)
    return &sliders->getAll();
  else
    return 0;
}  

void AllControls::setVersion(quint64 v) {
  vsn = v;
  if (vsn) {
    Adjustments a(Adjustments::fromDB(v, *db));
    QSize s(db->originalSize(vsn));
    sliders->setAll(a);
    if (cropper)
      cropper->setAll(a, s);
    setEnabled(true);
  } else {
    setEnabled(false);
  }
}

void AllControls::changeFromSliders() {
  Adjustments adj = sliders->getAll();
  emit valuesChanged(vsn, adj);
  if (cropper)
    cropper->setAll(adj);
}

void AllControls::changeFromCropper(QRect croprect, QSize osize) {
  Adjustments adj0 = sliders->getAll();
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

  emit valuesChanged(vsn, adj);
}

QSize AllControls::sizeHint() const {
  return QTabWidget::sizeHint();
  //  return sliders->sizeHint(); // a bit taller...
}

Actions const &AllControls::actions() {
  return ControlSliders::actions();
}
