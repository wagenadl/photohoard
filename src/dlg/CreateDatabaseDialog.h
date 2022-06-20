// CreateDatabaseDialog.h

#ifndef CREATEDATABASEDIALOG_H

#define CREATEDATABASEDIALOG_H

#include <QDialog>

class CreateDatabaseDialog: public QDialog {
  Q_OBJECT;
public:
  CreateDatabaseDialog(QWidget *parent=0);
  virtual ~CreateDatabaseDialog();
  void setup();
  void showEvent(QShowEvent *) override;
public slots:
  void browseLocation();
  void browseCache();
  void accept() override;
private:
  class Ui_createDatabaseDialog *ui;
};

#endif
