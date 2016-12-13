// Dialog.cpp

#include "Dialog.h"
#include "ScreenResolution.h"
#include <QWidget>

void Dialog::ensureSize(QWidget *dlg) {
  QSize current = dlg->size();
  QSize minhint = dlg->minimumSizeHint();
  QSize better = current;
  if (better.width() < minhint.width())
    better.setWidth(minhint.width());
  if (better.height() < minhint.height())
    better.setHeight(minhint.height());
  if (better != current)
    dlg->resize(better);
}
