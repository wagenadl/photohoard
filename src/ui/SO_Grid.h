// SO_Grid.h

#ifndef SO_GRID_H

#define SO_GRID_H

#include "SlideOverlay.h"

class SO_Grid: public SlideOverlay {
public:
  SO_Grid(class SlideView *parent=0);
  virtual void render(class QPainter *ptr, class QRect const &rect);
};

#endif
