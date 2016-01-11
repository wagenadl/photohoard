// SlideOverlay.h

#ifndef SLIDEOVERLAY_H

#define SLIDEOVERLAY_H

#include <QObject>

class SlideOverlay: public QObject {
  Q_OBJECT;
public:
  SlideOverlay(QObject *parent=0);
public:
  virtual void render(class QPainter *ptr, class QRect const &rect,
                      class QTransform const &imgToWidget,
                      quint64 versionid);
signals:
  void repaintNeeded();
};

#endif
