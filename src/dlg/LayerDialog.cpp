// LayerDialog.cpp

#include "LayerDialog.h"
#include "PDebug.h"
#include "ui_LayerDialog.h"
#include "PhotoDB.h"
#include "Layers.h"
#include <QTableWidget>
#include <QTableWidgetSelectionRange>

LayerDialog::LayerDialog(PhotoDB *db, QWidget *parent):
  QWidget(parent), db(db) {
  ui = new Ui_LayerDialog;
  ui->setupUi(this);
  setVersion(0);
}

void LayerDialog::setVersion(quint64 v) {
  vsn = v;
  Layers layers(vsn, db);
  int N = layers.count();

  ui->table->setRowCount(N+1);

  ui->table->setVerticalHeaderItem(N, new QTableWidgetItem("B"));
  ui->table->setItem(N, 0, new QTableWidgetItem(QString::fromUtf8("☼")));
  ui->table->setItem(N, 1, new QTableWidgetItem(QString::fromUtf8("-")));
  ui->table->setItem(N, 2,
		     new QTableWidgetItem(Layer::typeName(Layer::Type::Base)));

  for (int n=1; n<=N; n++) {
    Layer layer = layers.layer(n);
    ui->table->setVerticalHeaderItem(N-n,
				   new QTableWidgetItem(QString("%1").arg(n)));
    ui->table->setItem(N-n, 0, new QTableWidgetItem(layer.isActive()
						    ? QString::fromUtf8("☼")
						    : QString::fromUtf8("☀")));
    ui->table->setItem(N-n, 1, new QTableWidgetItem(QString::fromUtf8("-")));
    ui->table->setItem(N-n, 2, new QTableWidgetItem(layer.typeName()));
  }
  lastlay = 0;
  ui->table->selectRow(N);
}

LayerDialog::~LayerDialog() {
}

void LayerDialog::addGradientLayer() {
  pDebug() << "addGradientLayer";
  Layers ll(vsn, db);
  Layer l;
  l.setType(Layer::Type::LinearGradient);
  PSize osize = db->originalSize(vsn);
  QPoint p0(osize.width()/2, osize.height()/3);
  QPoint p1(osize.width()/2, osize.height()*2/3);
  QPolygon pp; pp << p0 << p1;
  l.setPoints(pp);
  ll.addLayer(l);
  emit edited(ll.count());

  setVersion(vsn); // rebuild
  ui->table->selectRow(0); // select the new layer
}

void LayerDialog::addLayer() {
  qDebug() << "addlayer";
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

  emit edited(lay);
  
  setVersion(vsn);
  pDebug() << "deleted layer" << lay << lastlay;
  emit layerSelected(0);
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
  emit edited(lay);

  setVersion(vsn); // rebuild
  lay ++;
  ui->table->selectRow(N - lay); // select the new layer
}

void LayerDialog::lowerLayer() {
  int lay = selectedLayer();
  if (lay==0) {
    COMPLAIN("CANNOT RAISE BASE LAYER");
    return;
  }
  Layers ll(vsn, db);
  int N = ll.count();
  if (lay==1) {
    COMPLAIN("CANNOT LOWER BOTTOM LAYER");
    return;
  }
  ll.lowerLayer(lay);
  emit edited(lay-1);

  setVersion(vsn); // rebuild
  lay --;
  ui->table->selectRow(N - lay); // select the new layer
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
  emit edited(lay);

  ui->table->item(ui->table->rowCount()-1-lay, 0)
    ->setText(l.isActive()
	      ? QString::fromUtf8("☼")
	      : QString::fromUtf8("☀"));
}

void LayerDialog::showHideMask() {
  pDebug() << "NYI";
}

void LayerDialog::newSelection() {
  int lay = selectedLayer();
  pDebug() << "new selection" << lay << lastlay;
  if (lay!=lastlay) {
    lastlay = lay;
    emit layerSelected(lay);
  }
  ui->del->setEnabled(lay>0);
  ui->raise->setEnabled(lay>0 && lay<Layers(vsn,db).count());
  ui->lower->setEnabled(lay>1);
  ui->showHide->setEnabled(lay>0);
  ui->mask->setEnabled(lay>0);
}
 
int LayerDialog::selectedLayer() const {
  int rows = ui->table->rowCount();
  auto range = ui->table->selectedRanges();
  int row = range.isEmpty() ? rows-1
    : range.first().topRow();
  return rows - 1 - row;
}

void LayerDialog::respondToClick(int r, int c) {
  pDebug() << "click" << r << c;
}
