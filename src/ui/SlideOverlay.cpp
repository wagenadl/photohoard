// SlideOverlay.cpp

#include "SlideOverlay.h"
#include "PDebug.h"

SlideOverlay::SlideOverlay(SlideView *base): QObject(base), sv(base) {
}

SlideView *SlideOverlay::base() {
  SlideView *b = sv;
  ASSERT(b);
  return b;
}
