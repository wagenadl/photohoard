// Action.h

#ifndef ACTION_H

#define ACTION_H

#include <functional>
#include <QKeySequence>
#include <QList>
#include <vector>

class Action {
public:
  Action(int key, QString doc, std::function<void()> foo);
  Action(int key, QString doc, class QAction *act);
  Action(std::vector<int> const &keys, QString doc, std::function<void()> foo);
  Action(QString pseudokey, QString doc);
  void activate() const;
  bool activateIf(QKeySequence const &key) const;
  QKeySequence key() const;
  QString keyName() const;
  QString documentation() const;
public:
  QList<QKeySequence> keys;
  QString pseudokey;
  QString doc;
  std::function<void()> foo;
  class QAction *act;
};

class Actions {
public:
  Actions &operator<<(Action const &);
  bool activateIf(class QKeyEvent *);
private:
  QList<Action> acts;
};

#endif
