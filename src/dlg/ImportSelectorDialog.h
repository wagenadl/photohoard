// ImportSelectorDialog.h

#ifndef IMPORTSELECTORDIALOG_H

#define IMPORTSELECTORDIALOG_H

#include <QDialog>

class ImportSelectorDialog: public QDialog {
  Q_OBJECT;
public:
  ImportSelectorDialog(QStringList choices, QWidget *parent=0);
  virtual ~ImportSelectorDialog();
  static QString choose(QStringList choices);
  QString choice() const { return choice_; }
private slots:
  void select();
  void cancel();
private:
  class Ui_ImportSelectorDialog *ui;
  QStringList choices;
  QString choice_;
};

#endif
