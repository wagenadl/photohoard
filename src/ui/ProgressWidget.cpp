// ProgressWidget.cpp

#include "ProgressWidget.h"

ProgressWidget::ProgressWidget(QString title, QWidget *parent):
  QLabel(parent), title(title) {
  setObjectName("ProgressWidget_" + title);
  setText(title);
}

void ProgressWidget::markProgress(int n, int N) {
  setText(QString("%1: %2/%3").arg(title).arg(n).arg(N));
}
