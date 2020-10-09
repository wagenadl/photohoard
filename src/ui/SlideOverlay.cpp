// SlideOverlay.cpp

#include "SlideOverlay.h"
#include "PDebug.h"

SlideOverlay::SlideOverlay(SlideView *base): QWidget(base), sv(base) {
  setAttribute(Qt::WA_TranslucentBackground);
  resize(base->size());
  show();
}

SlideView const *SlideOverlay::base() const {
  SlideView const *b = sv;
  ASSERT(b);
  return b;
}

SlideView *SlideOverlay::base() {
  SlideView *b = sv;
  ASSERT(b);
  return b;
}
