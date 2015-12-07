// AC_ImageHolder.cpp

#include "AC_ImageHolder.h"
#include <QMutexLocker>

AC_ImageHolder::AC_ImageHolder(QObject *parent): QObject(parent) {
}

void AC_ImageHolder::setImage(quint64 id, Image16 img) {
  QMutexLocker m(&mutex);
  hold[id] = img;
}

void AC_ImageHolder::dropImage(quint64 id) {
  QMutexLocker m(&mutex);
  if (hold.contains(id))
    hold.remove(id);
}

Image16 AC_ImageHolder::getImage(quint64 id) {
  QMutexLocker m(&mutex);
  if (hold.contains(id)) {
    Image16 res = hold[id];
    hold.remove(id);
    return res;
  } else {
    return Image16();
  }
}
