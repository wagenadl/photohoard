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
  Action(int key, QString doc, std::function<void()> foo);
  /* - Constructor with a single key shortcut and a functor
     ACTION(key, doc, foo) constructs a new ACTION with shortcut KEY,
     documentation text DOC, and payload FOO. KEY can be any Qt::Key_XXX
     constant, optionally modified with Qt::CTRL, Qt::SHIFT, or Qt::ALT.
     FOO can be any functor. It is acceptable to create an undocumented
     action by leaving DOC empty.
     Example:
         Action{Qt::Key_F1, "Quit", []() { QApplication::quit(); }}
  */
  Action(int key, QString doc, class QAction *act);
  /* - Constructor with a single key shortcut and a QAction
     ACTION(key, doc, act) constructs a  new ACTION with shortcut KEY,
     documentation text DOC, and payload ACT. When the action is activated
     (through ACTIVATE or ACTIVATEIF), the QAction is triggered.
     The payload is automatically modified to be activated by the shortcut
     and DOC is set as its TEXT. (If DOC contains newlines, the part before
     the first newline is used for its TEXT, and the whole DOC is set as
     its tooltip.) In addition, a text represention of the KEY is added
     to the text and tooltip.
  */
  Action(std::vector<int> const &keys, QString doc, std::function<void()> foo);
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
public:
  QList<QKeySequence> keys;
  QString pseudokey;
  QString doc;
  std::function<void()> foo;
  class QAction *act;
};

class PQAction: public QAction {
  /* PQAction - Self-sufficient QAction
     A PQAction is a QAction with a payload that gets executed when the
     action is triggered.
  */
  Q_OBJECT;
public:
  PQAction(std::function<void()> foo, QObject *parent=0);
  /* - Constructor
     PQAction(foo) constructs a new PQAction with payload FOO, which may
     be any functor. The PQAction behaves exactly like a regular QAction,
     except that it additionally calls the payload when the action is
     triggered.
     PQAction(foo, parent) is the same but with a PARENT object.
     A PQACTION may be conveniently documented through the ACTION/ACTIONS
     system.
     Technical note: This is implemented by connecting to the action's
     TRIGGERED signal, so DISCONNECT()ing all signals disables its function.
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
  bool activateIf(class QKeyEvent *);
  /* ACTIVATEIF - Execute the payload of an appropriate action
     ACTIVATEIF(event) iterates through the ACTION objects in the collection
     until one is found that has a shortcut matching the KEY and MODIFIERS
     encoded in the EVENT.
     If one is found, it is executed, and ACTIVATEIF returns TRUE. Otherwise,
     it returns FALSE.
   */
private:
  QList<Action> acts;
};

#endif
