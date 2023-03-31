// CollapseButton.cpp

#include "CollapseButton.h"

/* This entire class inspired by
   https://stackoverflow.com/questions/32476006/how-to-make-an-expandable-collapsable-section-widget-in-qt
 */

CollapseButton::CollapseButton(QWidget *parent):
  QLabel(parent) {
  _shown = true;
  _target = 0;
}

CollapseButton::CollapseButton(QString txt, QWidget *parent):
  CollapseButton(parent) {
  setText(txt);
}

CollapseButton::~CollapseButton() {
}

void CollapseButton::setText(QString const &text) {
  _text = text;
  QLabel::setText((_shown ? "▾ " : "▸ ") + _text);
}

void CollapseButton::showTarget() {
  _shown = true;
  if (_target)
    _target->show();
  setText(_text);
}

void CollapseButton::hideTarget() {
  _shown = false;
  if (_target)
    _target->hide();
  setText(_text);
}

void CollapseButton::setTarget(QWidget *target) {
  _target = target;
}

void CollapseButton::mousePressEvent(QMouseEvent *e) {
  if (_shown)
    hideTarget();
  else
    showTarget();
}
