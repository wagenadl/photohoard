// Messenger.h

#ifndef MESSENGER_H

#define MESSENGER_H

#include <QObject>
#include <QPointer>
#include <QMap>

class Messenger: public QObject {
  /* MESSENGER - Basic user message system */
  Q_OBJECT;
public:
  Messenger(class MainWindow *parent);
  /* The constructor creates the messenger. To send a message, see below. */
  virtual ~Messenger();
  static void message(QObject *src, QString msg, double timeout_s=10);
  /* MESSAGE - Send a message
     MESSAGE(src, msg) sends a message to the user interface. SRC must
     specify a QObject that is a descendant of a main window, otherwise
     the message will only go to the terminal.
     MESSAGE(src, msg, timeout_s) specifies for how many seconds the
     message will be shown. (The default is 10 s.)
     The message will also be removed when another message is sent from
     the same source.
  */
  static void message(QObject *src, QString id, QString msg,
                      double timeout_s=10);
  /* MESSAGE - Send a message
     MESSAGE(src, id, msg) is just like MESSAGE(src, msg), except that
     ID can be used to further identify the message. The message will
     stay visible until another message with that same ID is sent, or
     after timeout.
  */
  static void removeMessage(QObject *source, QString id="");
  /* REMOVEMESSAGE - Remove a message
     REMOVEMESSAGE(src) removes any messages currently visible that are
     associated with SRC.
  */
private:
  void sendMessage(QString id, QString msg, double timeout_s);
private:
  class MainWindow *owner;
private:
  static QMap<QObject *, QPointer<Messenger> > &messengers();
};

#endif
