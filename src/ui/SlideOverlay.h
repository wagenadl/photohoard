// SlideOverlay.h

#ifndef SLIDEOVERLAY_H

#define SLIDEOVERLAY_H

#include <QObject>
#include "SlideView.h"
#include <QPainter>
#include <QPointer>

class SlideOverlay: public QObject {
  Q_OBJECT;
public:
  SlideOverlay(class SlideView *parent=0);
public:
  virtual void render(class QPainter *ptr, class QRect const &rect)=0;
  /* RENDER - Render this overlay
     RENDER(ptr, rect) requests that the rectangle RECT (in widget coordinates)
     be repainted with whatever overlay is appropriate for the version
     currently shown in the parent SLIDEVIEW.
  */
protected:
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
