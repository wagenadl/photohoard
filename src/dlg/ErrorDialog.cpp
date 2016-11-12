// ErrorDialog.cpp

#include "ErrorDialog.h"
#include <QMessageBox>

void ErrorDialog::fatal(QString msg) {
  QMessageBox::critical(0, "Photohoard", msg);
}
