// ExportDialog.h

#ifndef EXPORTDIALOG_H

#define EXPORTDIALOG_H

#include <QDialog>
#include "ExportSettings.h"
#include <QDialogButtonBox>

class ExportDialog: public QDialog {
  Q_OBJECT;
public:
  ExportDialog(bool now=false, QWidget *parent=0);
  virtual ~ExportDialog();
  void setup(ExportSettings const &);
  ExportSettings settings() const;
  DialogCode exec();
public:
  static void showandexport(class Exporter *expo, bool now=false);
public slots:
  void setFormat(int);
  void setResolutionMode();
  void browse();
  void handleClick(class QAbstractButton *x);
private:
  class Ui_exportDialog *ui;
  class PhotoDB *db;
  QDialogButtonBox::ButtonRole br;
};

#endif
