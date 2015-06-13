// SliderMask.cpp

#include "SliderMask.h"

void SliderMask::enable(QString key, bool on) {
  if (universe().contains(key)) {
    if (on)
      keys.insert(key);
    else
      keys.remove(key);
  }
}

void SliderMask::disable(QString key, bool off) {
  enable(key, !off);
}

bool SliderMask::isEnabled(QString key) const {
  return keys.contains(key);
}

QSet<QString> SliderMask::mask() const {
  return keys;
}

QSet<QString> const &SliderMask::universe() {
  static QSet<QString> u;
  if (u.isEmpty()) {
#define SLIDER(name, dfl) u.insert(#name);
#include "sliders.def"
#undef SLIDER
  }
  return u;
}
