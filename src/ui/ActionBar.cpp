// ActionBar.cpp

#include "ActionBar.h"
#include "ScreenResolution.h"
#include <math.h>
#include "PDebug.h"

ActionBar::ActionBar(QWidget *parent): QToolBar(parent) {
  double dpi = ScreenResolution().dpi();
  double height_mm = ScreenResolution().millimeterSize().height();
  double heightCorr = pow(height_mm / 200, .3);
  double isize_inch = 0.17 * heightCorr;
  int isize = int(isize_inch*dpi + 0.5);
  setIconSize(QSize(isize, isize));
}

Actions const &ActionBar::actions() const {
  return acts;
}
