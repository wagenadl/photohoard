// RootsList.cpp

#include "RootsList.h"
#include "PhotoDB.h"
#include "ui_RootsList.h"

RootsList::RootsList(PhotoDB *db, QWidget *parent): QDialog(parent) {
  ui = new Ui_RootsList();
  ui->setupUi(this);
  QStringList roots(db->rootFolders());
  ui->list->addItems(roots);
}

RootsList::~RootsList() {
}

  
