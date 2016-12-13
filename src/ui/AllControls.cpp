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

  layers = new LayerDialog(db);
  addTab(layers, QIcon(":/icons/layers.svg"), "Layers");
  connect(layers, SIGNAL(layerSelected(int)),
	  SLOT(setLayer(int)));
  connect(layers, SIGNAL(edited(int)),
	  SLOT(layersEdited(int)));
  connect(this, SIGNAL(currentChanged(int)),
	  SLOT(changeOfIndex()));
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
  adjs.clear();
  vsn = v;
  lay = 0;
  if (vsn) {
    layers->setVersion(v);
    Adjustments a(Adjustments::fromDB(v, *db));
    adjs[0] = a;
    // Other layers?
    QSize s(db->originalSize(vsn));
    sliders->setAll(a);
    sliders->setLayer(0);
    if (cropper) {
      setTabEnabled(1, lay==0);
      cropper->setAll(a, s);
    }
    setEnabled(true);
  } else {
    setEnabled(false);
  }
}

void AllControls::changeFromSliders() {
  Adjustments adj = sliders->getAll();
  storeInDatabase(adj);
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
  storeInDatabase(adj);
}

void AllControls::setLayer(int l) {
  lay = l;
  qDebug() << "setLayer" << lay;

  if (!adjs.contains(lay))
    adjs[lay] = Adjustments::fromDB(vsn, lay, *db);
  sliders->setAll(adjs[lay]);
  sliders->setLayer(lay);
  if (cropper) {
    cropper->setAll(adjs[lay]);
    setTabEnabled(1, lay==0);
  }

  if (currentWidget()==layers)
    emit layerSelected(vsn, lay);
}

void AllControls::storeInDatabase(Adjustments const &adj) {
  Untransaction t(db);
  quint64 layid = lay
    ? Layers(vsn, db).layerID(lay)
    : 0;
  for (QString k: Adjustments::keys()) {
    double v = adj.get(k);
    if (v != adjs[lay].get(k)) {
      db->addUndoStep(vsn, lay, k, adjs[lay].get(k), v);
      if (lay==0) {
	if (v==Adjustments::defaultFor(k))
	  db->query("delete from adjustments where version==:a and k==:b",
		    vsn, k);
	else
	  db->query("insert or replace into adjustments (version, k, v)"
		    " values (:a, :b, :c)", vsn, k, v);
      } else {
	if (v==Adjustments::defaultFor(k))
	  db->query("delete from layeradjustments where layer==:a and k==:b",
		    layid, k);
	else
	  db->query("insert or replace into layeradjustments (layer, k, v)"
		    " values (:a, :b, :c)", layid, k, v);
      }
    }
  }
  adjs[lay] = adj;
  emit valuesChanged(vsn, lay, adj);
}  

QSize AllControls::sizeHint() const {
  return QTabWidget::sizeHint();
  //  return sliders->sizeHint(); // a bit taller...
}

Actions const &AllControls::actions() {
  return ControlSliders::actions();
}

void AllControls::changeOfIndex() {
  if (currentWidget()==layers)
    emit layerSelected(vsn, layers->selectedLayer());
  else
    emit layerSelected(vsn, 0);
}

void AllControls::layersEdited(int lowest) {
  pDebug() << "Layers changed" << vsn;
  emit layersChanged(vsn, lowest);
}
