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
signals:
  void apply();
private slots: 
  friend class Ui_FilterDialog;
  void recount();
  void setMaker();
  void setCamera();
  void recolorTags();
  void buttonClicked(QAbstractButton *);
protected:
  virtual void showEvent(QShowEvent *) override;
private:
  void prepCombos();
  void prepCollections();
  void prepCameras();
  void prepMakes();
  void prepModels(QString make="");
  void prepLenses(QString make="", QString camera="");
private:
  class Ui_FilterDialog *ui;
  PhotoDB db;
};

#endif
