// ColorLabelBar.cpp

#include "ColorLabelBar.h"

ColorLabelBar::ColorLabelBar(QWidget *parent): QToolBar(parent) {
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

  addAction(actions[Action::SetNone]);
  addAction(actions[Action::SetRed]);
  addAction(actions[Action::SetYellow]);
  addAction(actions[Action::SetGreen]);
  addAction(actions[Action::SetBlue]);
  addAction(actions[Action::SetPurple]);

  connect(this, SIGNAL(actionTriggered(QAction*)),
          SLOT(trigger(QAction*)));
}

ColorLabelBar::~ColorLabelBar() {
}

void ColorLabelBar::trigger(QAction *a) {
  if (revmap.contains(a))
    emit revmap[a];
}

