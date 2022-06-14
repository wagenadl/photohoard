// DatabaseDialog.h

#ifndef DATABASEDIALOG_H

#define DATABASEDIALOG_H

#include <QDialog>
#include <QWidget>
#include "SessionDB.h"
#include <QModelIndex>

class DatabaseDialog: public QDialog {
  Q_OBJECT;
public:
  DatabaseDialog(SessionDB *sdb, QWidget *parent=0);
  virtual ~DatabaseDialog();
  void setup();
  static QString niceSize(quint64 bytesize);
  static quint64 dirSize(QString root);
  static QString niceCount(quint64 count);
public slots:
  void moveDB();
  void moveSession();
  void moveCache();
  void renameDB();
  void createNew();
  void openOther();
  void openRecent(QModelIndex);
  void showRoots();
private:
  class Ui_databaseDialog *ui;
  class SessionDB *sdb;
};

#endif
