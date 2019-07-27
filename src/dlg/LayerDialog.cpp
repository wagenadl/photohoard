// LayerDialog.cpp

#include "LayerDialog.h"
#include "PDebug.h"
#include "ui_LayerDialog.h"
#include "PhotoDB.h"
#include "Layers.h"
#include <QTableWidget>
#include <QTableWidgetSelectionRange>
#include "Dialog.h"
#include "ControlSliders.h"

LayerDialog::LayerDialog(PhotoDB *db, QWidget *parent):
  QWidget(parent), db(db) {
  ui = new Ui_LayerDialog;
  ui->setupUi(this);
  //  setVersion(0);
  sliders = new ControlSliders(db->isReadOnly(), 0);
  sliders->setLayer(1); // make sure not to show recompose group
  ui->verticalLayout->addWidget(sliders);
  Dialog::ensureSize(this);
  connect(sliders, SIGNAL(valuesChanged()),
	  SLOT(changeFromSliders()));
}

void LayerDialog::setVersion(quint64 v) {
  vsn = v;
  adjs.clear();
  if (vsn==0) {
    ui->table->setRowCount(0);
    sliders->setAll(Adjustments());
    sliders->setEnabled(false);
    return;
  }
  
  Layers layers(vsn, db);
  int N = layers.count();

  ui->table->setRowCount(N);

  for (int n=1; n<=N; n++) {
    Layer layer = layers.layer(n);
    //pDebug() << "layerdialog::setversion" << v << n
    //         << int(layer.type()) << layer.points();
    ui->table->setVerticalHeaderItem(N-n,
				   new QTableWidgetItem(QString("%1").arg(n)));
    ui->table->setItem(N-n, 0, new QTableWidgetItem(layer.isActive()
						    ? QString::fromUtf8("☼")
						    : QString::fromUtf8("☀")));
    ui->table->setItem(N-n, 1, new QTableWidgetItem(QString::fromUtf8("-")));
    ui->table->setItem(N-n, 2, new QTableWidgetItem(layer.typeName()));
  }
  bool explicitnew = lastlay==N;
  //pDebug() << "layerdialog setversion explicitnew" << explicitnew;
  lastlay = 0;
  selectLayer(N);
  if (explicitnew)
    newSelection();
}

LayerDialog::~LayerDialog() {
}

void LayerDialog::selectLayer(int lay) {
  ui->table->selectionModel()->clearSelection();
  if (lay>0)
    ui->table->selectRow(ui->table->rowCount() - lay);
}


void LayerDialog::addGradientLayer() {
  //pDebug() << "addGradientLayer";
  Layers ll(vsn, db);
  Layer l;
  l.setType(Layer::Type::LinearGradient);
  PSize osize = db->originalSize(vsn);
  QPoint p0(osize.width()/2, osize.height()/3);
  QPoint p1(osize.width()/2, osize.height()*2/3);
  QPolygon pp; pp << p0 << p1;
  l.setPoints(pp);
  ll.addLayer(l);
  emit maskEdited(ll.count());

  setVersion(vsn); // rebuild
}

void LayerDialog::addLayer() {
  addGradientLayer();
}

void LayerDialog::deleteLayer() {
  int lay = selectedLayer();
  if (lay==0) {
    COMPLAIN("Cannot delete base layer");
    return;
  }

  Layers ll(vsn, db);
  ll.deleteLayer(lay);

  emit maskEdited(lay);
  
  setVersion(vsn);
  //  pDebug() << "deleted layer" << lay << lastlay;
  selectLayer(Layers(vsn, db).count());
}

void LayerDialog::raiseLayer() {
  int lay = selectedLayer();
  if (lay==0) {
    COMPLAIN("CANNOT RAISE BASE LAYER");
    return;
  }
  Layers ll(vsn, db);
  int N = ll.count();
  if (lay==N) {
    COMPLAIN("CANNOT RAISE TOP LAYER");
    return;
  }
  ll.raiseLayer(lay);
  emit maskEdited(lay);

  setVersion(vsn); // rebuild
  lay ++;
  selectLayer(lay);
}

void LayerDialog::lowerLayer() {
  int lay = selectedLayer();
  if (lay==0) {
    COMPLAIN("CANNOT RAISE BASE LAYER");
    return;
  }
  Layers ll(vsn, db);
  if (lay==1) {
    COMPLAIN("CANNOT LOWER BOTTOM LAYER");
    return;
  }
  ll.lowerLayer(lay);
  emit maskEdited(lay-1);

  setVersion(vsn); // rebuild
  lay --;
  selectLayer(lay);
}

void LayerDialog::showHideLayer() {
  int lay = selectedLayer();
  if (lay==0) {
    COMPLAIN("CANNOT SHOW/HIDE BASE LAYER");
    return;
  }
  Layers ll(vsn, db);
  Layer l = ll.layer(lay);
  l.setActive(!l.isActive());
  ll.setLayer(lay, l);
  emit maskEdited(lay);

  ui->table->item(ui->table->rowCount()-lay, 0)
    ->setText(l.isActive()
	      ? QString::fromUtf8("☼")
	      : QString::fromUtf8("☀"));
}

void LayerDialog::showHideMask() {
  COMPLAIN("show/hide mask NYI");
}

void LayerDialog::newSelection() {
  int lay = selectedLayer();
  //  pDebug() << "layer dialog new selection" << lay << lastlay;
  if (lay!=lastlay) {
    lastlay = lay;
    emit layerSelected(lay);
  }
  if (lay>0) {
    if (!adjs.contains(lay))
      adjs[lay] = Adjustments::fromDB(vsn, lay, *db);
    sliders->setAll(adjs[lay]);
  } else {
    sliders->setAll(Adjustments());
  }
  sliders->setEnabled(lay>0);
  ui->del->setEnabled(lay>0);
  ui->raise->setEnabled(lay>0 && lay<Layers(vsn,db).count());
  ui->lower->setEnabled(lay>1);
  ui->showHide->setEnabled(lay>0);
  ui->mask->setEnabled(lay>0);
}
 
int LayerDialog::selectedLayer() const {
  int rows = ui->table->rowCount();
  auto range = ui->table->selectedRanges();
  //pDebug() << "selectedlayer"
  //	   << (range.isEmpty() ? 0 : rows - range.first().topRow())
  //	   << "out of" << rows << range.isEmpty();
  if (range.isEmpty())
    return 0;
  return rows - range.first().topRow();
}

void LayerDialog::respondToClick(int r, int c) {
  //pDebug() << "click" << r << c;
  switch (c) {
  case 0: // visibility
    showHideLayer();
    break;
  case 1: // mask
    showHideMask();
    break;
  default:
    break;
  }
}

void LayerDialog::changeFromSliders() {
  pDebug() << "LayerDialog::changeFromSliders";
  int lay = selectedLayer();
  if (!lay)
    return;
  Adjustments adj = sliders->getAll();
  storeInDatabase(adj, lay);
}

void LayerDialog::storeInDatabase(Adjustments const &adj, int lay) {
  if (!lay)
    return;
  Untransaction t(db);
  quint64 layid = Layers(vsn, db).layerID(lay);
  Adjustments const &a0(adjs[lay]);
  for (QString k: Adjustments::keys()) {
    double v = adj.get(k);
    double v0 = a0.get(k);
    if (v != v0) {
      db->addUndoStep(vsn, lay, k, v0, v);
      if (v==Adjustments::defaultFor(k))
        db->query("delete from layeradjustments where layer==:a and k==:b",
                  layid, k);
      else
        db->query("insert or replace into layeradjustments (layer, k, v)"
                  " values (:a, :b, :c)", layid, k, v);
    }
  }
  adjs[lay] = adj;
  emit valuesChanged(lay);
}  

Adjustments const *LayerDialog::getAll(quint64 v, int lay) const {
  if (v!=vsn)
    return 0;
  auto it(adjs.find(lay));
  if (it==adjs.end())
    return 0;
  return &*it;
}
