// ActionBar.cpp

#include "ActionBar.h"
#include <QSignalMapper>

ActionBar::ActionBar(QWidget *parent): QToolBar(parent), parent(parent) {
  mapper = new QSignalMapper(this);
  connect(mapper, SIGNAL(mapped(QObject *)), SLOT(mtrigger(QObject *)));
  connect(this, SIGNAL(actionTriggered(QAction*)), SLOT(trigger(QAction*)));
}

ActionBar::~ActionBar() {
}

void ActionBar::addHiddenAction(QAction *a) {
  parent->addAction(a);
  connect(a, SIGNAL(triggered(bool)), mapper, SLOT(map()));
  mapper->setMapping(a, a);
}  

void ActionBar::mtrigger(QObject *a) {
  trigger(dynamic_cast<QAction*>(a));
}
