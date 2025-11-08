// AllControls.cpp

#include "AllControls.h"
#include "Action.h"
#include "ControlSliders.h"
#include "CropControls.h"
#include "LayerDialog.h"
#include "PhotoDB.h"
#include "Layers.h"
#include "PDebug.h"
#include <QVBoxLayout>

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

  QWidget *sa = new QWidget();
  auto *lay = new QVBoxLayout();
  lay->setContentsMargins(2, 2, 2, 2);
  sa->setLayout(lay);
  sa->setObjectName("layerdialog");
  layers = new LayerDialog(db);
  lay->addWidget(layers);
  addTab(sa, QIcon(":/icons/layers.svg"), "Layers");
  connect(layers, SIGNAL(layerSelected(int)),
	  SLOT(layerIndexChange(int)));
  connect(layers, SIGNAL(maskEdited(int)),
	  SLOT(maskChangeFromLayers(int)));
  connect(layers, SIGNAL(valuesChanged(int)),
	  SLOT(valueChangeFromLayers(int)));
  connect(this, SIGNAL(currentChanged(int)),
	  SLOT(changeOfTabIndex()));
  connect(layers, &LayerDialog::inSliders,
          this, &AllControls::inSliders);
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
  if (currentWidget()==layers) {
    emit layerSelected(vsn, lay);
  }
}

void AllControls::storeInDatabase(Adjustments const &adj) {
  bool locked = false;
  for (QString k: Adjustments::keys()) {
    double v = adj.get(k);
    double v0 = adjs.get(k);
    if (v != v0) {
      if (!locked) {
        locked = true;
        db->lockForWriting();
      }
      db->addUndoStep(vsn, k, v0, v);
      if (v==Adjustments::defaultFor(k))
        db->query("delete from adjustments where version==:a and k==:b",
                  vsn, k);
      else
        db->query("insert or replace into adjustments (version, k, v)"
                  " values (:a, :b, :c)", vsn, k, v);
    }
  }
  if (locked)
    db->unlockForWriting();
  adjs = adj;
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
  emit maskChanged(vsn, lay);
}

void AllControls::valueChangeFromLayers(int lay) {
  Adjustments const *adj = layers->getAll(vsn, lay);
  if (adj) {
    emit valuesChanged(vsn, lay, *adj);
  }
}
