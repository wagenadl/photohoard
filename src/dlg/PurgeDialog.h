// PurgeDialog.h

#ifndef PURGEDIALOG_H

#define PURGEDIALOG_H

#include <QDialog>

class PurgeDialog: public QDialog {
  Q_OBJECT;
public:
  PurgeDialog(class PhotoDB *, class Purge const *);
  virtual ~PurgeDialog();
protected slots:
  void restoreClicked();
  void deleteToggled();
protected:
  void keyPressEvent(QKeyEvent *);
public:
  static void purgeDialog(class PhotoDB *, class AutoCache *);
private:
  class PhotoDB *db;
  class Purge const *purge;
  class Ui_PurgeDialog *ui;
};

#endif
