// ClickLabel.cpp

#include "ClickLabel.h"
#include <QMouseEvent>

ClickLabel::ClickLabel(QWidget *parent): QLabel(parent) {
}

ClickLabel::ClickLabel(QString txt, QWidget *parent): QLabel(txt, parent) {
}

ClickLabel::~ClickLabel() {
}

void ClickLabel::mousePressEvent(QMouseEvent *e) {
  if (e->button()==Qt::LeftButton)
    emit clicked();
  e->accept();
}
