// SlideOverlay.h

#ifndef SLIDEOVERLAY_H

#define SLIDEOVERLAY_H

#include <QWidget>
#include "SlideView.h"
#include <QPainter>
#include <QPointer>

class SlideOverlay: public QWidget {
  Q_OBJECT;
public:
  SlideOverlay(class SlideView *parent=0);
protected:
  SlideView const *base() const;
  SlideView *base();
  /* BASE - SlideView that we are drawing on top of.
     BASE() returns the SlideView that this overlay lies over. Use this to
     find out the ID or size of the version being shown, or the transformation
     matrices needed.
  */
private:
  QPointer<SlideView> sv;
};

#endif
