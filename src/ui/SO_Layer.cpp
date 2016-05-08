// SO_Layer.cpp

#include "SO_Layer.h"

SO_Layer::SO_Layer(SlideView *sv): SlideOverlay(sv) { }

void SO_Layer::setLayer(Layer const &l) {
  lay = l;
  base()->update();
}

void SO_Layer::setTransform(Adjustments const &) {
  // hmmm.
}

void SO_Layer::render(QPainter *ptr, QRect const &) {
  //  QTransform const &xf = base()->transformationFromImage();
}

