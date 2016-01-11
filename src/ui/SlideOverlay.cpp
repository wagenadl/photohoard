// SlideOverlay.cpp

#include "SlideOverlay.h"
#include <QPainter>
#include "PDebug.h"

SlideOverlay::SlideOverlay(QObject *parent): QObject(parent) {
}


void SlideOverlay::render(QPainter *ptr, QRect const &rect,
                          QTransform const &imgToWidget,
                          quint64 versionid) {
}

