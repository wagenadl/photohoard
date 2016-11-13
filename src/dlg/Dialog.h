// Dialog.h

#ifndef DIALOG_H

#define DIALOG_H

#include <QDialog>

class Dialog: public QDialog {
public:
  static void ensureSize(QWidget *dlg);
public:
  Dialog(QWidget *parent=0);
protected:
  void ensureSize();
};

#endif
