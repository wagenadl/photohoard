// FilterDialog.h

#ifndef FILTERDIALOG_H

#define FILTERDIALOG_H

#include <QDialog>
#include "Filter.h"
#include "PhotoDB.h"

class FilterDialog: public QDialog {
  Q_OBJECT;
public:
  FilterDialog(class PhotoDB *db, QWidget *parent=0);
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
  void browseTags();
  void browseFolders();
  void buttonClick(class QAbstractButton *);
  void selectAllLabels();
protected:
  virtual void showEvent(QShowEvent *) override;
private:
  void prepCombos();
  void prepCollections();
  void prepCameras();
  void prepMakes();
  void prepModels(QString make="");
  void prepLenses(QString make="", QString camera="");
  void prepFolderTree();
  QStringList splitTags() const;
  void buildTree(class QTreeWidgetItem *it, quint64 parentid);
private:
  class Ui_FilterDialog *ui;
  PhotoDB *db;
  Filter f0;
  bool starting;
};

#endif
