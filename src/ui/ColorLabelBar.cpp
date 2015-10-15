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

  actions[Action::SetNoColor]->setIcon(QIcon(":icons/colorNone.svg"));
  actions[Action::SetRed]->setIcon(QIcon(":icons/colorRed.svg"));
  actions[Action::SetYellow]->setIcon(QIcon(":icons/colorYellow.svg"));
  actions[Action::SetGreen]->setIcon(QIcon(":icons/colorGreen.svg"));
  actions[Action::SetBlue]->setIcon(QIcon(":icons/colorBlue.svg"));
  actions[Action::SetPurple]->setIcon(QIcon(":icons/colorPurple.svg"));

  actions[Action::SetNoColor]->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_0));
  actions[Action::SetRed]->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_1));
  actions[Action::SetYellow]->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_2));
  actions[Action::SetGreen]->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_3));
  actions[Action::SetBlue]->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_4));
  actions[Action::SetPurple]->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_5));

  addAction(actions[Action::SetNoColor]);
  addAction(actions[Action::SetRed]);
  addAction(actions[Action::SetYellow]);
  addAction(actions[Action::SetGreen]);
  addAction(actions[Action::SetBlue]);
  addAction(actions[Action::SetPurple]);

  actions[Action::Set0Stars]->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT
							+ Qt::Key_0));
  actions[Action::Set1Star]->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT
						       + Qt::Key_1));
  actions[Action::Set2Stars]->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT
							+ Qt::Key_2));
  actions[Action::Set3Stars]->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT
							+ Qt::Key_3));
  actions[Action::Set4Stars]->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT
							+ Qt::Key_4));
  actions[Action::Set5Stars]->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT
							+ Qt::Key_5));

  addHiddenAction(actions[Action::Set0Stars]);
  addHiddenAction(actions[Action::Set1Star]);
  addHiddenAction(actions[Action::Set2Stars]);
  addHiddenAction(actions[Action::Set3Stars]);
  addHiddenAction(actions[Action::Set4Stars]);
  addHiddenAction(actions[Action::Set5Stars]);

  addHiddenAction(actions[Action::SetUndecided]);
  addHiddenAction(actions[Action::SetAccept]);
  addHiddenAction(actions[Action::SetReject]);
  
  actions[Action::RotateLeft]->setShortcut(QKeySequence(Qt::CTRL
							 + Qt::Key_Comma));
  actions[Action::RotateRight]->setShortcut(QKeySequence(Qt::CTRL
							  + Qt::Key_Period));
  
  addHiddenAction(actions[Action::RotateLeft]);
  addHiddenAction(actions[Action::RotateRight]);
}

ColorLabelBar::~ColorLabelBar() {
}

void ColorLabelBar::trigger(QAction *a) {
  if (revmap.contains(a))
    emit triggered(revmap[a]);
}

