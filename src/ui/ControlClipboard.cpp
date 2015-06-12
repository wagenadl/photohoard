// ControlClipboard.cpp

#include "ControlClipboard.h"

ControlClipboard::ControlClipboard(QWidget *parent) {
}

ControlClipboard::~ControlClipboard() {
}

Sliders ControlClipboard::values() const {
  return val;
}

QSet<QString> ControlClipboard::mask() const {
}

void ControlClipboard::get(Sliders *dest) const {
}

void ControlClipboard::set(class Sliders const &vv) {
}

void ControlClipboard::setAll(class Sliders const &vv) {
}
 // ignores mask
void ControlClipboard::setMask(QSet<QString>) {
}

void ControlClipboard::enableAll(bool on=true) {
}

void ControlClipboard::disableAll(bool off=true) {
}

void ControlClipboard::enableGroup(QString name, bool on=true) {
}

void ControlClipboard::disableGroup(QString name, bool off=true) {
}

void ControlClipboard::enable(QString name, bool on=true) {
}

void ControlClipboard::disable(QString name, bool off=true) {
}

void ControlClipboard::goNext(QString) {
}

void ControlClipboard::goPrevious(QString) {
}


