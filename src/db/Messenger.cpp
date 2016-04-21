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

void Messenger::message(QObject *src, QString msg, double) {
  QObject *top = src;
  while (src) {
    src = src->parent();
    if (src)
      top = src;
  }
  Messenger *msgr = messengers()[top];
  if (msgr) {
    MainWindow *mw = msgr->owner;
    if (mw) {
      mw->setStatusMessage(msg);
    }
  }
  qDebug() << "Message: " << msg;
}

void Messenger::message(QObject *src, QString, QString msg, double) {
  message(src, msg);
}
