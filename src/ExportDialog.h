// ExportDialog.h

#ifndef EXPORTDIALOG_H

#define EXPORTDIALOG_H

#include <QDialog>

class ExportDialog: public QDialog {
  Q_OBJECT;
public:
  ExportDialog(QWidget *parent=0);
  virtual ~ExportDialog();
private:
  class Ui_exportDialog *ui;
};

#endif
