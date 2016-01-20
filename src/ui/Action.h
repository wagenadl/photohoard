// Action.h

#ifndef ACTION_H

#define ACTION_H

#include <functional>
#include <QKeySequence>
#include <QList>
#include <QAction>
#include <vector>

class Action {
  // ACTION - Self-documenting functor with a key shortcut
  /* An ACTION is essentially a C++ 2011-style functor (anonymous function
     or lambda expression) with an associated key shortcut (or set of
     shortcuts) and a documentation string.
   */
public:
  Action(unsigned int key, QString doc, std::function<void()> foo);
  /* - Constructor with a single key shortcut and a functor
     ACTION(key, doc, foo) constructs a new ACTION with shortcut KEY,
     documentation text DOC, and payload FOO. KEY can be any Qt::Key_XXX
     constant, optionally modified with Qt::CTRL, Qt::SHIFT, or Qt::ALT.
     FOO can be any functor. It is acceptable to create an undocumented
     action by leaving DOC empty.
     Example:
         Action{Qt::Key_F1, "Quit", []() { QApplication::quit(); }}
  */
  Action(std::vector<unsigned int> const &keys, QString doc,
         std::function<void()> foo);
  /* - Constructor with a set of key shortcuts and a functor
     ACTION(keys, doc, foo) constructs a new ACTION with multiple shortcuts,
     listed in KEYS, documentation text DOC, and payload FOO.
     Example:
         Action{{Qt::Key_F1, Qt::CTRL | Qt::Key_Q}, "Quit", ...}
  */
  Action(QString pseudokey, QString doc);
  /* - Constructor without an action
     ACTION(pseudokey, doc) registers a documented action with an actual
     payload. This is useful for documentation purposes. E.g., you could
     construct undocumented actions for each of the arrow keys and then
     create a single pseudo-action by something like
         Action{"Arrows", "Navigate"}
   */
  void activate() const;
  /* ACTIVATE - Calls the payload 
     ACTIVATE() calls the payload if one is defined.
   */
  bool activateIf(QKeySequence const &key) const;
  /* ACTIVATEIF - Calls the payload if key sequence matches
     ACTIVATEIF(key), where KEY is a key sequence (usually a single key)
     calls the payload if KEY matches any of the shortcuts encoded with the
     action.
   */
  QKeySequence shortcut() const;
  /* SHORTCUT - Keyboard shortcut for this action
     SHORTCUT() returns the key sequence of the shortcut for this action.
     If multiple shortcuts are defined, the first one is used.
  */
  QList<QKeySequence> const &shortcuts() const;
  /* SHORTCUTS - Keyboard shortcuts for this action
     SHORTCUTS() returns all the key sequence that are set as shortcuts
     for this action.
  */
  QString keyName() const;
  /* KEYNAME - Text representation of keyboard shortcut
     KEYNAME() returns a textual representation (e.g. “Control+A”) for the
     shortcut for this action.
     If multiple shortcuts are defined, the first one is used.
     Returns the pseudo key for an action defined with a pseudo key.
  */
  QString documentation() const;
  /* DOCUMENTATION - Documentation string
     DOCUMENTATION() returns the documentation string set for this action
     at construction time.
  */
  std::function<void()> payload() const;
  /* PAYLOAD - Return the payload */
public:
  QList<QKeySequence> keys;
  QString pseudokey;
  QString doc;
  std::function<void()> foo;
};

class PAction: public QAction {
  /* PAction - QAction that activates an Action.
     A PAction is just like a QAction, except that when the QAction is
     triggered, the payload of the Action is activated. */
  Q_OBJECT;
public:
  PAction(Action const &a, QObject *parent=0);
  /* - Constructor
     PAction(act) constructs a new PAction based on the Action ACT.
     PAction(act, parent) specifies a parent object.
   */
  PAction(Action const &a, QIcon const &icon, QWidget *parent);
  /* - Constructor
     PAction(act, icon, parent) constructs a new PAction based on the
     Action ACT and adds it to the given PARENT widget with the given ICON.
   */
private slots:
  void activ8();
private:
  std::function<void()> foo;
};

class Actions {
  /* ACTIONS - A collection of individual ACTION objects */     
public:
  Actions &operator<<(Action const &);
  /* OPERATOR<< - Add an ACTION to the collection */
  bool activateIf(class QKeyEvent *) const;
  /* ACTIVATEIF - Execute the payload of an appropriate action
     ACTIVATEIF(event) iterates through the ACTION objects in the collection
     until one is found that has a shortcut matching the KEY and MODIFIERS
     encoded in the EVENT.
     If one is found, it is executed, and ACTIVATEIF returns TRUE. Otherwise,
     it returns FALSE.
   */
  Action const &last() const;
  QList<Action> const &all() const;
  Actions operator+(Actions const &) const;
private:
  QList<Action> acts;
};

#endif
