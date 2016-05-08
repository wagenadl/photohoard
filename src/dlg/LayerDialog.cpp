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

void LayerDialog::setVersion(quint64 vsn) {
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
  emit edited();
}

void LayerDialog::addLayer() {
  addGradientLayer();
}

void LayerDialog::deleteLayer() {
  emit edited();
}

void LayerDialog::raiseLayer() {
  emit edited();
}

void LayerDialog::lowerLayer() {
  emit edited();
}

void LayerDialog::showHideLayer() {
  emit edited();
}

void LayerDialog::showHideMask() {
  pDebug() << "NYI";
}

void LayerDialog::newSelection() {
  pDebug() << "new selection";
  int lay = selectedLayer();
  if (lay!=lastlay) {
    lastlay = lay;
    emit layerSelected(lay);
  }
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
