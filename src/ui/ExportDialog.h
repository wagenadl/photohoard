// ExportDialog.h

#ifndef EXPORTDIALOG_H

#define EXPORTDIALOG_H

#include <QDialog>
#include "ExportSettings.h"

class ExportDialog: public QDialog {
  Q_OBJECT;
public:
public:
  ExportDialog(class PhotoDB *db, QWidget *parent=0);
  virtual ~ExportDialog();
  void setup(ExportSettings const &);
  ExportSettings settings() const;
  DialogCode exec();
  bool everOKd() const;
public slots:
  void setFormat(int);
  void setResolutionMode();
  void browse();
private:
  class Ui_exportDialog *ui;
  class PhotoDB *db;
  bool ever_okd;
};

#endif
