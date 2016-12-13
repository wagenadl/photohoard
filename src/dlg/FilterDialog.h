// FilterDialog.h

#ifndef FILTERDIALOG_H

#define FILTERDIALOG_H

#include <QDialog>
#include "Filter.h"

class FilterDialog: public QDialog {
  Q_OBJECT;
public:
  FilterDialog(class SessionDB *db, QWidget *parent=0);
  virtual ~FilterDialog() { }
  Filter extract() const; // currently shown in dialog
public slots:
  void populate();
signals:
  void applied();
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
  class SessionDB *db;
  bool starting;
};

#endif
