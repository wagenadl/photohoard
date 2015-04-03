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
  Filter extract() const; // currently shown in dialog
  Filter const &filter() const { return f0; } // accepted or applied
signals:
  void apply();
private slots: 
  friend class Ui_FilterDialog;
  void recount();
  void setMaker();
  void setCamera();
  void recolorTags();
  void buttonClick(class QAbstractButton *);
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
  Filter f0;
};

#endif
