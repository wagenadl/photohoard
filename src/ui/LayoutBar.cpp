// LayoutBar.cpp

#include "LayoutBar.h"
#include <QMetaType>
#include <QDebug>
#include <QSignalMapper>

LayoutBar::LayoutBar(QWidget *parent): ActionBar(parent) {
  qRegisterMetaType<LayoutBar::Action>("LayoutBar::Action");

  setWindowTitle("Layout");
  
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

  actions[Action::FullGrid]->setText("Full grid (F1)");
  actions[Action::HGrid]->setText("Horizontal grid plus photo (Shift-F2)");
  actions[Action::VGrid]->setText("Vertical grid plus photo (F2)");
  actions[Action::HLine]->setText("Horizontal line plus photo (Shift-F3)");
  actions[Action::VLine]->setText("Vertical line plus photo (F3)");
  actions[Action::FullPhoto]->setText("Photo only (F4)");
  // etcetera

  actions[Action::FullGrid]->setShortcut(QString("F1"));
  actions[Action::HGrid]->setShortcut(QString("Shift+F2"));
  actions[Action::VGrid]->setShortcut(QString("F2"));
  actions[Action::HLine]->setShortcut(QString("Shift+F3"));
  actions[Action::VLine]->setShortcut(QString("F3"));
  actions[Action::FullPhoto]->setShortcut(QString("F4"));

  actions[Action::ToggleFullScreen]->setShortcut(QString("F5"));
  // etcetera
  
  addAction(actions[Action::FullGrid]);
  addAction(actions[Action::HGrid]);
  addAction(actions[Action::VGrid]);
  addAction(actions[Action::HLine]);
  addAction(actions[Action::VLine]);
  addAction(actions[Action::FullPhoto]);

  addHiddenAction(actions[Action::ToggleFullScreen]);
}

LayoutBar::~LayoutBar() {
}

void LayoutBar::trigger(QAction *a) {
  qDebug() << "LayoutBar::trigger" << a << revmap.contains(a);
  if (revmap.contains(a))
    emit triggered(revmap[a]);
}
