// FilterBar.cpp

#include "FilterBar.h"

#include <QMetaType>
#include <QDebug>

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
  actions[Action::Smaller]->setText("Smaller (Control-Minus)");
  actions[Action::Larger]->setText("Larger (Control-Plus)");

  actions[Action::OpenFilterDialog]
    ->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F));
  actions[Action::Smaller]
    ->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Minus));
  actions[Action::Larger]
    ->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Plus));
  actions[Action::ClearSelection]
    ->setShortcuts(QList<QKeySequence>()
                   << QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_A)
                   << QKeySequence(Qt::Key_Escape)
                   );
  addAction(actions[Action::Smaller]);
  addAction(actions[Action::Larger]);
  addAction(actions[Action::OpenFilterDialog]);

  addHiddenAction(actions[Action::ClearSelection]);
}

FilterBar::~FilterBar() {
}


void FilterBar::trigger(QAction *a) {
  qDebug() << "FilterBar::trigger" << a << revmap.contains(a);
  if (revmap.contains(a))
    emit triggered(revmap[a]);
}

