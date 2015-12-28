// ActionBar.cpp

#include "ActionBar.h"

ActionBar::ActionBar(QWidget *parent): QToolBar(parent) {
}

Actions const &ActionBar::actions() const {
  return acts;
}
