// InstaSlot.cpp

#include "InstaSlot.h"

InstaSlot::InstaSlot(QObject *source, char const *sig,
                     QObject *dest, std::function<void()> foo,
                     Qt::ConnectionType type): QObject(dest), foo(foo) {
  connect(source, sig, SLOT(slot()), type);
}

InstaSlot::InstaSlot(QObject *source, char const *sig,
                     std::function<void()> foo,
                     Qt::ConnectionType type): QObject(source), foo(foo) {
  connect(source, sig, SLOT(slot()), type);
}

void InstaSlot::slot() {
  foo();
}

//////////////////////////////////////////////////////////////////////

InstaBool::InstaBool(QObject *source, char const *sig,
                     QObject *dest, std::function<void(bool)> foo,
                     Qt::ConnectionType type): QObject(dest), foo(foo) {
  connect(source, sig, SLOT(slot(bool)), type);
}

InstaBool::InstaBool(QObject *source, char const *sig,
                     std::function<void(bool)> foo,
                     Qt::ConnectionType type): QObject(source), foo(foo) {
  connect(source, sig, SLOT(slot(bool)), type);
}

void InstaBool::slot(bool b) {
  foo(b);
}

//////////////////////////////////////////////////////////////////////

InstaInt::InstaInt(QObject *source, char const *sig,
                     QObject *dest, std::function<void(int)> foo,
                     Qt::ConnectionType type): QObject(dest), foo(foo) {
  connect(source, sig, SLOT(slot(int)), type);
}

InstaInt::InstaInt(QObject *source, char const *sig,
                     std::function<void(int)> foo,
                     Qt::ConnectionType type): QObject(source), foo(foo) {
  connect(source, sig, SLOT(slot(int)), type);
}

void InstaInt::slot(int i) {
  foo(i);
}


//////////////////////////////////////////////////////////////////////

InstaString::InstaString(QObject *source, char const *sig,
                     QObject *dest, std::function<void(QString)> foo,
                     Qt::ConnectionType type): QObject(dest), foo(foo) {
  connect(source, sig, SLOT(slot(QString)), type);
}

InstaString::InstaString(QObject *source, char const *sig,
                     std::function<void(QString)> foo,
                     Qt::ConnectionType type): QObject(source), foo(foo) {
  connect(source, sig, SLOT(slot(QString)), type);
}

void InstaString::slot(QString s) {
  foo(s);
}
