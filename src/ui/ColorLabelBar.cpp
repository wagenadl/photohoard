// ColorLabelBar.cpp

#include "ColorLabelBar.h"

ColorLabelBar::ColorLabelBar(QWidget *parent): ActionBar(parent) {
  qRegisterMetaType<ColorLabelBar::Action>("ColorLabelBar::Action");

  setWindowTitle("Color label");
  
  for (int i=0; i<int(Action::N); i++) {
    Action ii = Action(i);
    QAction *a = new QAction(parent);
    actions[ii] = a;
    revmap[a] = ii;
  }

  actions[Action::SetNone]->setIcon(QIcon(":icons/colorNone.svg"));
  actions[Action::SetRed]->setIcon(QIcon(":icons/colorRed.svg"));
  actions[Action::SetYellow]->setIcon(QIcon(":icons/colorYellow.svg"));
  actions[Action::SetGreen]->setIcon(QIcon(":icons/colorGreen.svg"));
  actions[Action::SetBlue]->setIcon(QIcon(":icons/colorBlue.svg"));
  actions[Action::SetPurple]->setIcon(QIcon(":icons/colorPurple.svg"));

  actions[Action::SetNone]->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_0));
  actions[Action::SetRed]->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_1));
  actions[Action::SetYellow]->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_2));
  actions[Action::SetGreen]->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_3));
  actions[Action::SetBlue]->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_4));
  actions[Action::SetPurple]->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_5));

  addAction(actions[Action::SetNone]);
  addAction(actions[Action::SetRed]);
  addAction(actions[Action::SetYellow]);
  addAction(actions[Action::SetGreen]);
  addAction(actions[Action::SetBlue]);
  addAction(actions[Action::SetPurple]);
}

ColorLabelBar::~ColorLabelBar() {
}

void ColorLabelBar::trigger(QAction *a) {
  if (revmap.contains(a))
    emit triggered(revmap[a]);
}

