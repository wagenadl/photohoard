// ExportDialog.h

#ifndef EXPORTDIALOG_H

#define EXPORTDIALOG_H

#include <QDialog>
#include "ExportSettings.h"

class ExportDialog: public QDialog {
  Q_OBJECT;
public:
  ExportDialog(bool now=false, QWidget *parent=0);
  virtual ~ExportDialog();
  void setup(ExportSettings const &);
  ExportSettings settings() const;
  int exec();
public:
  static void standalone(class Exporter *expo, bool now=false);
public slots:
  void setFormat(int);
  void setResolutionMode();
  void browse();
  void reset();
  void exportNow();
private:
  class Ui_exportDialog *ui;
  class PhotoDB *db;
  bool do_export;
};

#endif
