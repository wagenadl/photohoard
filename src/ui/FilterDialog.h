// FilterDialog.h

#ifndef FILTERDIALOG_H

#define FILTERDIALOG_H

#include <QDialog>

class FilterDialog: public QDialog {
  Q_OBJECT;
public:
  FilterDialog(QWidget *parent=0);
  virtual ~FilterDialog() { }
public slots:
  void recount();
  void setMaker();
  void setCamera();
  void recolorTags();
protected:
  virtual void showEvent(QEvent *) override;
private:
  class Ui_FilterDialog *ui;
};

#endif
