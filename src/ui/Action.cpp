// Action.cpp

#include "Action.h"
#include <QKeyEvent>
#include <QAction>

Action::Action(int key, QString doc, std::function<void()> foo):
  doc(doc), foo(foo), act(0) {
  keys << QKeySequence(key);
}

Action::Action(int key, QString doc, QAction *act):
  doc(doc), act(act) {
  keys << QKeySequence(key);
  act->setShortcut(keys.first());
  int idx = doc.indexOf("\n");
  if (idx>=0) {
    act->setText(doc.left(idx) + " (" + keys.first().toString() + ")");
    act->setToolTip(doc + " (" + keys.first().toString() + ")");
  } else {
    act->setText(doc + " (" + keys.first().toString() + ")");
  }
}

Action::Action(std::vector<int> const &kk,
               QString doc, std::function<void()> foo):
  doc(doc), foo(foo), act(0) {
  for (auto k: kk) 
    keys << QKeySequence(k);
}


Action::Action(QString pseudokey, QString doc):
  pseudokey(pseudokey), doc(doc), act(0) {
}

QKeySequence Action::shortcut() const {
  return keys.isEmpty() ? QKeySequence(): keys.first();
}

QString Action::keyName() const {
  if (keys.isEmpty())
    return pseudokey;
  else
    return shortcut().toString();
}

QString Action::documentation() const {
  return doc;
}

void Action::activate() const {
  if (act) 
    act->activate(QAction::Trigger);
  else if (foo)
    foo();
}

bool Action::activateIf(QKeySequence const &key) const {
  for (auto &k: keys) {
    if (key.matches(k)==QKeySequence::ExactMatch) {
      activate();
      return true;
    }
  }

  return false;
}


PQAction::PQAction(std::function<void()> foo, QObject *parent):
  QAction(parent), foo(foo) {
  connect(this, SIGNAL(triggered()),
          this, SLOT(activ8()));
}

void PQAction::activ8() {
  if (foo)
    foo();
}

Actions &Actions::operator<<(Action const &a) {
  acts << a;
  return *this;
}

bool Actions::activateIf(QKeyEvent *e) {
  int k = e->key();
  if (e->modifiers() & Qt::ControlModifier)
    k |= Qt::ControlModifier;
  if (e->modifiers() & Qt::AltModifier)
    k |= Qt::AltModifier;
  if (e->modifiers() & Qt::ShiftModifier)
    k |= Qt::ShiftModifier;
  QKeySequence seq(k);

  for (auto &a: acts) 
    if (a.activateIf(seq))
      return true;

  return false;
}
