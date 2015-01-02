// LayoutBar.cpp

#include "LayoutBar.h"

LayoutBar::LayoutBar(QWidget *parent): QToolBar(parent) {
  for (int i=0; i<int(Action::N); i++) {
    Action ii = Action(i);
    QAction *a = new QAction(parent);
    actions[ii] = a;
    revmap[a] = ii;
  }

  actions[Action::FullGrid]->setIcon(QIcon(":icons/layoutGrid.svg"));
  actions[Action::HGrid]->setIcon(QIcon(":icons/layoutHGrid.svg"));
  actions[Action::VGrid]->setIcon(QIcon(":icons/layoutVGrid.svg"));
  actions[Action::HLine]->setIcon(QIcon(":icons/layoutHLine.svg"));
  actions[Action::VLine]->setIcon(QIcon(":icons/layoutVLine.svg"));
  actions[Action::FullPhoto]->setIcon(QIcon(":icons/layoutFull.svg"));
  // etcetera

  actions[Action::FullGrid]->setText("Grid");
  actions[Action::HGrid]->setText("Hor. grid");
  actions[Action::VGrid]->setText("Vert. grid");
  actions[Action::HLine]->setText("Hor. line");
  actions[Action::VLine]->setText("Vert. line");
  actions[Action::FullPhoto]->setText("Photo only");
  // etcetera

  actions[Action::ToggleFullScreen]->setShortcut(QString("F5"));
  // etcetera
  
  addAction(actions[Action::FullGrid]);
  addAction(actions[Action::HGrid]);
  addAction(actions[Action::VGrid]);
  addAction(actions[Action::HLine]);
  addAction(actions[Action::VLine]);
  addAction(actions[Action::FullPhoto]);

  connect(this, SIGNAL(actionTriggered(QAction*)),
          SLOT(trigger(QAction*)));
}

LayoutBar::~LayoutBar() {
}

void LayoutBar::trigger(QAction *a) {
  if (revmap.contains(a))
    emit revmap[a];
}

