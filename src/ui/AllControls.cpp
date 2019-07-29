// AllControls.cpp

#include "AllControls.h"
#include "Action.h"
#include "ControlSliders.h"
#include "CropControls.h"
#include "LayerDialog.h"
#include "PhotoDB.h"
#include "Layers.h"
#include "PDebug.h"

AllControls::AllControls(PhotoDB *db, QWidget *parent):
  QTabWidget(parent), db(db) {
  vsn = 0;

  bool ro = db->isReadOnly();
  
  sliders = new ControlSliders(ro);
  addTab(sliders, QIcon(":/icons/sliders.svg"), "Base");
  connect(sliders, SIGNAL(valuesChanged()),
	  SLOT(changeFromSliders()));

  cropper = new CropControls();
  addTab(cropper, QIcon(":/icons/crop.svg"), "Crop");
  connect(cropper, SIGNAL(rectangleChanged(QRect, QSize)),
          SLOT(changeFromCropper(QRect, QSize)));
  if (ro) 
    cropper->setEnabled(false);

  layers = new LayerDialog(db);
  addTab(layers, QIcon(":/icons/layers.svg"), "Layers");
  connect(layers, SIGNAL(layerSelected(int)),
	  SLOT(layerIndexChange(int)));
  connect(layers, SIGNAL(maskEdited(int)),
	  SLOT(maskChangeFromLayers(int)));
  connect(layers, SIGNAL(valuesChanged(int)),
	  SLOT(valueChangeFromLayers(int)));
  connect(this, SIGNAL(currentChanged(int)),
	  SLOT(changeOfTabIndex()));
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
    layers->setVersion(v);
    adjs.readFromDB(v, *db);
    QSize s(db->originalSize(vsn));
    sliders->setAll(adjs);
    layers->setVersion(v);
    if (cropper) 
      cropper->setAll(adjs, s);
    setEnabled(true);
  } else {
    setEnabled(false);
    adjs.reset();
  }
}

void AllControls::changeFromSliders() {
  Adjustments adj = sliders->getAll();
  storeInDatabase(adj);
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
  storeInDatabase(adj);
}

void AllControls::layerIndexChange(int lay) {
  //qDebug() << "layerIndexChange" << lay;
  if (currentWidget()==layers) {
    emit layerSelected(vsn, lay);
  }
}

void AllControls::storeInDatabase(Adjustments const &adj) {
  Untransaction t(db);
  for (QString k: Adjustments::keys()) {
    double v = adj.get(k);
    double v0 = adjs.get(k);
    if (v != v0) {
      db->addUndoStep(vsn, k, v0, v);
      if (v==Adjustments::defaultFor(k))
        db->query("delete from adjustments where version==:a and k==:b",
                  vsn, k);
      else
        db->query("insert or replace into adjustments (version, k, v)"
                  " values (:a, :b, :c)", vsn, k, v);
    }
  }
  adjs = adj;
  pDebug() << "AllControls::storeInDatabase: emitting valuesChanged";
  emit valuesChanged(vsn, 0, adj);
}  

QSize AllControls::sizeHint() const {
  return QTabWidget::sizeHint();
  //  return sliders->sizeHint(); // a bit taller...
}

Actions const &AllControls::actions() {
  return ControlSliders::actions();
}

void AllControls::changeOfTabIndex() {
  if (currentWidget()==layers)
    emit layerSelected(vsn, layers->selectedLayer());
  else
    emit layerSelected(vsn, 0);
}

void AllControls::maskChangeFromLayers(int lay) {
  pDebug() << "mask change" << vsn << lay;
  emit maskChanged(vsn, lay);
}

void AllControls::valueChangeFromLayers(int lay) {
  Adjustments const *adj = layers->getAll(vsn, lay);
  if (adj) {
    pDebug() << "AllControls::valueChangeFromLayers: emitting valuesChanged";
    emit valuesChanged(vsn, lay, *adj);
  }
}
