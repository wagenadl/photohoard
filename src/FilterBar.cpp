// FilterBar.cpp

#include "FilterBar.h"

#include <QMetaType>
#include <QDebug>

FilterBar::FilterBar(QWidget *parent): QToolBar(parent) {
  qRegisterMetaType<FilterBar::Action>("FilterBar::Action");
  for (int i=0; i<int(Action::N); i++) {
    Action ii = Action(i);
    QAction *a = new QAction(parent);
    actions[ii] = a;
    revmap[a] = ii;
  }

  actions[Action::OpenFilterDialog]->setIcon(QIcon(":icons/search.svg"));
  actions[Action::ScaleSmaller]->setIcon(QIcon(":icons/scaleSmaller.svg"));
  actions[Action::ScaleLarger]->setIcon(QIcon(":icons/scaleLarger.svg"));

  actions[Action::OpenFilterDialog]->setText("Filter (Control-F)");
  actions[Action::ScaleSmaller]->setText("Smaller (Control-Minus)");
  actions[Action::ScaleLarger]->setText("Larger (Control-Plus)");

  actions[Action::OpenFilterDialog]->setShortcut(QString("Control+F"));
  actions[Action::ScaleSmaller]->setShortcut(QString("Control+Minus"));
  actions[Action::ScaleLarger]->setShortcut(QString("Control+Plus"));

  addAction(actions[Action::ScaleSmaller]);
  addAction(actions[Action::ScaleLarger]);
  addAction(actions[Action::OpenFilterDialog]);

  connect(this, SIGNAL(actionTriggered(QAction*)),
          SLOT(trigger(QAction*)));
}

FilterBar::~FilterBar() {
}

void FilterBar::trigger(QAction *a) {
  qDebug() << "FilterBar::trigger" << a << revmap.contains(a);
  if (revmap.contains(a))
    emit triggered(revmap[a]);
}
