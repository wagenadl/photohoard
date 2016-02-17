// BoldButton.cpp

#include "BoldButton.h"

BoldButton::BoldButton(QString txt, QWidget *parent):
  QPushButton(txt, parent) {
  setCheckable(true);
  connect(this, SIGNAL(toggled(bool)), SLOT(reflect()));
  reflect();
}

BoldButton::~BoldButton() {
}

QSize BoldButton::sizeHint() const {
  if (ss.isEmpty())
    ss = QPushButton::sizeHint();
  return ss;
}

void BoldButton::reflect() {
  QFont f;
  if (isChecked())
    f.setWeight(QFont::Bold);
  else
    f.setWeight(QFont::Normal);
  setFont(f);
}
