// RootsList.h

#ifndef ROOTSLIST_H

#define ROOTSLIST_H

#include <QDialog>

class RootsList: public QDialog {
public:
  RootsList(class PhotoDB *db, QWidget *parent=0);
  virtual ~RootsList();
private:
  class Ui_RootsList *ui;
};

#endif
