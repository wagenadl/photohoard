// Messenger.cpp

#include "Messenger.h"
#include "MainWindow.h"
#include "PDebug.h"

Messenger::Messenger(MainWindow *parent): QObject(parent), owner(parent) {
  ASSERT(parent);
  messengers()[parent] = this;
}

Messenger::~Messenger() {
  messengers().remove(owner);
}

QMap<QObject *, QPointer<Messenger> > &Messenger::messengers() {
  static QMap<QObject *, QPointer<Messenger> > msgrs;
  return msgrs;
}

void Messenger::message(QObject *src, QString id, QString msg, double t) {
  Messenger *msgr = 0;
  QObject *obj = src;
  while (obj) {
    if (messengers().contains(obj)) {
      msgr = messengers()[obj];
      break;
    } else {
      obj = obj->parent();
    }
  }
  if (msgr==0 && !messengers().isEmpty())
    msgr = *messengers().begin();

  if (msgr)
    msgr->sendMessage(QString("%1:%2").arg(reinterpret_cast<quint64>(src))
                      .arg(id), msg, t);
  else
    COMPLAIN("No way to send message: " + msg);
}

void Messenger::sendMessage(QString, QString msg, double) {
  ASSERT(owner);
  owner->setStatusMessage(msg);
}

void Messenger::message(QObject *src, QString msg, double t) {
  message(src, "", msg, t);
}
