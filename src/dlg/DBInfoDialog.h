// DbinfoDialog.h

#ifndef DBINFODIALOG_H

#define DBINFODIALOG_H

#include <QDialog>
#include <QWidget>
#include "SessionDB.h"
#include <QModelIndex>

class DBInfoDialog: public QDialog {
  Q_OBJECT;
public:
  DBInfoDialog(SessionDB *sdb, QWidget *parent=0);
  virtual ~DBInfoDialog();
  void setup();
  static QString niceSize(quint64 bytesize);
  static quint64 dirSize(QString root);
  static QString niceCount(quint64 count);
private:
  class Ui_DBInfoDialog *ui;
  class SessionDB *sdb;
  QString locationlabel;
};

#endif
