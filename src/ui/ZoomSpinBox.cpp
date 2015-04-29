// ZoomSpinBox.cpp

#include "ZoomSpinBox.h"

ZoomSpinBox::ZoomSpinBox(QWidget *parent): QSpinBox(parent) {
  minz = 0.05;
  maxz = 10;
}

double ZoomSpinBox::zoom() const {
  bool ok;
  QString txt = text();
  if (txt.endsWith("%"))
    txt = txt.left(txt.length()-1);
  double z = txt.toDouble(&ok);
  return ok ? z/100.0 : 1.0;
}  

void ZoomSpinBox::setZoom(double z) {
  if (z>=minz && z<=maxz)
    updateText(z);
  else if (z<minz)
    updateText(minz);
  else if (z>maxz)
    updateText(maxz);
}

void ZoomSpinBox::setMinZoom(double z1) {
  minz = z1;
  if (z<minz) {
    z = minz;
    updateText();
  }
  if (maxz<minz)
    maxz = minz;
}

void ZoomSpinBox::setMaxZoom(double z1) {
  maxz = z1;
  if (z>maxz) {
    z = maxz;
    updateText();
  }
  if (minz>maxz)
    minz = maxz;
}

void ZoomSpinBox::stepBy(int steps) {
  bool ok;
  double z = text().toDouble(&ok);
  if (ok)
    z = 
  while (steps>0) {
    z *= 1.55;
    double basez = pow(2, floor(log(z)/log(2)));
    if (basez>=1) {
      if (z>1.5*basez)
        z = 1.5*basez;
      else
        z = basez;
    } else {
      if (z>basez*2/1.5)
        z = 2/1.5 * basez;
      else
        z = basez;
    }
    steps--;
  }
  while (steps<0) {
    z /= 1.55;
    double basez = pow(2, ceil(log(z)/log(2)));
    if (basez>=2) {
      if (z<1.5/2*basez)
        z = 1.5/2*basez;
      else
        z = basez;
    } else {
      if (z<basez/1.5)
        z = basez/1.5;
      else
        z = basez;
    }
    steps++;
  }
  if (z>maxz)
    z = maxz;
  if (z<minz)
    z = minz;
  updateText();
}

void ZoomSpinBox::fixup(QString &input) const {
  /* This should make input be acceptable if it is not as-is.
   */
}

QValidator::State ZoomSpinBox::validate(QString &input, int &pos) const {
  /* This should just check whether input is acceptable, not change it.
   */
  QString x = input;
  if (x.endsWith("%"))
    x = x.left(x.length()-1);
  bool ok;
  double y = x.toDouble(&ok);
  if (!ok)
    return Invalid;
  else if (y>=minz && y<=maxz)
    return Acceptable;
  else
    return Intermediate;
}

ZoomSpinBox::StepEnabled ZoomSpinBox::stepEnabled() const {
  StepEnabled e = StepNone;
  if (z>minz)
    e |= StepDownEnabled;
  if (z<maxz)
    e |= StepUpEnabled;
  return e;
}
