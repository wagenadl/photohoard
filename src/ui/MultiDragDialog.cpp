// MultiDragDialog.cpp

#include "MultiDragDialog.h"
#include "ui_MultiDragDialog.h"

MultiDragDialog::MultiDragDialog(SessionDB *db,
                                 QSet<quint64> const &sel, quint64 id):
  db(db), sel(sel), id(id) {
  ui = new Ui_MultiDragDialog;
  ui->setupUi(this);
}

MultiDragDialog::~MultiDragDialog() {
}


