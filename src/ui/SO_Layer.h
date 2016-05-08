// SO_Layer.h

#ifndef SO_LAYER_H

#define SO_LAYER_H

#include "SlideOverlay.h"
#include "Layers.h"

class SO_Layer: public SlideOverlay {
public:
  SO_Layer(class SlideView *parent=0);
  void setLayer(Layer const &);
  void setTransform(class Adjustments const &);
  virtual void render(class QPainter *ptr, class QRect const &rect);
private:
  Layer lay;
};

#endif
