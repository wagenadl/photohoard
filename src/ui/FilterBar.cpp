// FilterBar.cpp

#include "FilterBar.h"

#include <QMetaType>
#include "PDebug.h"

FilterBar::FilterBar(QWidget *parent): ActionBar(parent) {
  qRegisterMetaType<FilterBar::Action>("FilterBar::Action");

  setWindowTitle("Filter");
  
  for (int i=0; i<int(Action::N); i++) {
    Action ii = Action(i);
    QAction *a = new QAction(parent);
    actions[ii] = a;
    revmap[a] = ii;
  }

  actions[Action::OpenFilterDialog]->setIcon(QIcon(":icons/search.svg"));
  actions[Action::Smaller]->setIcon(QIcon(":icons/scaleSmaller.svg"));
  actions[Action::Larger]->setIcon(QIcon(":icons/scaleLarger.svg"));

  actions[Action::OpenFilterDialog]->setText("Filter (Control-F)");
  actions[Action::Smaller]->setText("Smaller");
  actions[Action::Larger]->setText("Larger");

  actions[Action::OpenFilterDialog]
    ->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F));
  actions[Action::ClearSelection]
    ->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_A));
  actions[Action::SelectAll]
    ->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_A));

  addAction(actions[Action::Smaller]);
  addAction(actions[Action::Larger]);
  addAction(actions[Action::OpenFilterDialog]);
  addHiddenAction(actions[Action::ClearSelection]);
  addHiddenAction(actions[Action::SelectAll]);
}

FilterBar::~FilterBar() {
}


void FilterBar::trigger(QAction *a) {
  pDebug() << "FilterBar::trigger" << a << revmap.contains(a);
  if (revmap.contains(a))
    emit triggered(revmap[a]);
}

