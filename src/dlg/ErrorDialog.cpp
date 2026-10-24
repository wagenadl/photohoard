// ErrorDialog.cpp

#include "ErrorDialog.h"
#include <QMessageBox>
#include <QApplication>

void ErrorDialog::fatal(QString msg) {
  QWidget *parent = 0;
  for (QWidget *w : QApplication::allWidgets()) {
    if (w)
      parent = w->window();
    if (parent)
      break;
  }
  QMessageBox::critical(parent, "Photohoard", msg);
}
