// RestoreDialog.h

#ifndef RESTOREDIALOG_H

#define RESTOREDIALOG_H

#include <QDialog>
#include <QSet>

class RestoreDialog: public QDialog {
  Q_OBJECT;
public:
  RestoreDialog(int N, QWidget *parent=0);
  virtual ~RestoreDialog();
public:
  static void restoreDialog(class PhotoDB *db, QSet<quint64> const &photos);
private:
  class Ui_RestoreDialog *ui;
};

#endif
