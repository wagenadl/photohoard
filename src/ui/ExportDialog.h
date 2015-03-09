// ExportDialog.h

#ifndef EXPORTDIALOG_H

#define EXPORTDIALOG_H

#include <QDialog>
#include "ExportSettings.h"

class ExportDialog: public QDialog {
  Q_OBJECT;
public:
public:
  ExportDialog(QWidget *parent=0);
  virtual ~ExportDialog();
  void setup(ExportSettings const &);
  ExportSettings settings() const;
  DialogCode exec();
public slots:
  void setFormat(int);
  void setResolutionMode();
private:
  class Ui_exportDialog *ui;
};

#endif
