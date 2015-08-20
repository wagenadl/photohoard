// AddRootDialog.h

#ifndef ADDROOTDIALOG_H

#define ADDROOTDIALOG_H

#include <QDialog>
#include <QModelIndex>

class AddRootDialog: public QDialog {
  Q_OBJECT;
public:
  AddRootDialog(class PhotoDB *db, QWidget *parent=0);
  virtual ~AddRootDialog();
  DialogCode exec();
  QString path() const;
  QString defaultCollection() const;
  QStringList exclusions() const;
protected:
  void keyPressEvent(QKeyEvent *) override;
protected slots:
  void browse();
  void addExclusion();
  void removeExclusion();
  void editExclusion(QModelIndex);
private:
  void prepCollections();
private:
  class Ui_addRootDialog *ui;
  class PhotoDB *db;
};

#endif
