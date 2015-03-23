// FilterDialog.h

#ifndef FILTERDIALOG_H

#define FILTERDIALOG_H

#include <QDialog>
#include "Filter.h"
#include "PhotoDB.h"

class FilterDialog: public QDialog {
  Q_OBJECT;
public:
  FilterDialog(class PhotoDB const &db, QWidget *parent=0);
  virtual ~FilterDialog() { }
  void populate(Filter const &);
  Filter extract() const;
public slots:
  void recount();
  void setMaker();
  void setCamera();
  void recolorTags();
protected:
  virtual void showEvent(QShowEvent *) override;
private:
  class Ui_FilterDialog *ui;
  PhotoDB db;
};

#endif
