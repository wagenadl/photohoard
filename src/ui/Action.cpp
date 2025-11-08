// Action.cpp

#include "Action.h"
#include <QKeyEvent>
#include <QAction>
#include "PDebug.h"
#include <QWidget>

Action::Action(QKeyCombination key, QString doc, std::function<void()> foo):
  doc(doc), foo(foo) {
  keys << QKeySequence(key);
}

Action::Action(QList<QKeyCombination> const &kk,
               QString doc, std::function<void()> foo):
  doc(doc), foo(foo) {
  for (auto k: kk) 
    keys << QKeySequence(k);
}

Action::Action(QString pseudokey, QString doc):
  pseudokey(pseudokey), doc(doc) {
}

QKeySequence Action::shortcut() const {
  return keys.isEmpty() ? QKeySequence(): keys.first();
}

QList<QKeySequence> const &Action::shortcuts() const {
  return keys;
}

QString Action::keyName() const {
  if (keys.isEmpty())
    return pseudokey;
  else
    return shortcut().toString();
}

std::function<void()> Action::payload() const {
  return foo;
}

QString Action::documentation() const {
  return doc;
}

void Action::activate() const {
  if (foo)
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

//////////////////////////////////////////////////////////////////////

PAction::PAction(Action const &a, QObject *parent):
  QAction(parent), foo(a.payload()) {
  connect(this, SIGNAL(triggered()), this, SLOT(activ8()));
  setShortcuts(a.shortcuts());
  QString doc = a.documentation();
  QString key = " (" + a.keyName() + ")";
  int idx = doc.indexOf("\n");
  if (idx>=0) {
    setText(doc.left(idx)); // + key);
    setToolTip(doc + key);
  } else {
    setText(doc); // + key);
    setToolTip(doc + key);
  }
}

PAction::PAction(Action const &a, QIcon const &icon, QWidget *parent):
  PAction(a, parent) {
  setIcon(icon);
  parent->addAction(this);
}  

void PAction::activ8() {
  if (foo) 
    foo();
}

//////////////////////////////////////////////////////////////////////

Actions &Actions::operator<<(Action const &a) {
  acts << a;
  return *this;
}

bool Actions::activateIf(QKeyEvent *e) const {
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

Action const &Actions::last() const {
  Q_ASSERT(!acts.isEmpty());
  return acts.last();
}

QList<Action> const &Actions::all() const {
  return acts;
}

Actions Actions::operator+(Actions const &b) const {
  Actions a = *this;
  a.acts.append(b.acts);
  return a;
}
