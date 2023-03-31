// FirstrunDialog.h

#ifndef FIRSTRUNDIALOG_H

#define FIRSTRUNDIALOG_H

#include <QDialog>

class FirstRunDialog: public QDialog {
  Q_OBJECT;
public:
  FirstRunDialog(QWidget *parent=0);
  virtual ~FirstRunDialog();
  QStringList roots() const;
public slots:
  void add();
  void drop();
private:
  class Ui_FirstRunDialog *ui;
};

#endif
