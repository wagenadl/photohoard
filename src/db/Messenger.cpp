// Messenger.cpp

#include "Messenger.h"
#include "MainWindow.h"
#include "PDebug.h"

Messenger::Messenger(MainWindow *parent): QObject(parent) {
  ASSERT(parent);
  messengers()[parent] = this;
  connect(this, SIGNAL(posted(QString)),
          parent, SLOT(setStatusMessage(QString)),
          Qt::QueuedConnection);
}

Messenger::~Messenger() {
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
  if (!msgr) {
    for (auto m: messengers()) {
      msgr = m;
      if (msgr)
        break;
    }
  }
  
  if (msgr)
    msgr->sendMessage(QString("%1:%2").arg(reinterpret_cast<quint64>(src))
                      .arg(id), msg, t);
  else
    COMPLAIN("No way to send message: " + msg);
}

void Messenger::sendMessage(QString, QString msg, double) {
  emit posted(msg);
}

void Messenger::message(QObject *src, QString msg, double t) {
  message(src, "", msg, t);
}
