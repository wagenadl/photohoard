// AddRootDialog.h

#ifndef ADDROOTDIALOG_H

#define ADDROOTDIALOG_H

#include <QDialog>

class AddRootDialog: public QDialog {
  Q_OBJECT;
public:
  AddRootDialog(class PhotoDB *db, QWidget *parent=0);
  virtual ~AddRootDialog();
  DialogCode exec();
  QString path() const;
  QString defaultCollection() const;
protected:
  void keyPressEvent(QKeyEvent *) override;
protected slots:
  void browse();
private:
  void prepCollections();
private:
  class Ui_addRootDialog *ui;
  class PhotoDB *db;
};

#endif
