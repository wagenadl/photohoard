// VScrollArea.cpp

#include "VScrollArea.h"
#include <QScrollBar>

VScrollArea::VScrollArea(QWidget *parent): QFrame(parent) {
  vscroll = new QScrollBar(this);
}

void VScrollArea::setChild(QWidget *c) {
  if (c==child)
    return;
  if (child)
    child->removeEventFilter(this);
  if (c) {
    setSizePolicy(c->horizontalSizePolicy(), QSizePolicy::Expanding);
    c->installEventFilter(this);
  }
  updateGeometry();
}

bool VScrollArea::eventFilter(QObject *obj, QEvent *evt) {
  if (obj==child && evt->type()==QEvent::Resize)
    childResized(child->size());
  return QFrame::eventFilter(obj, evt);
}

QSize VScrollArea::sizeHint() const{
  int w = 0;
  if (child)
    w += child->width();
  if (vscroll->isVisible())
    w += vscroll->width();
  int h = (child) ? child->height() : 20;
  return QSize(w, h);
}

QSize VScrollArea::minimumSizeHint() const {
  return QSize(sizeHint().width(), 20);
}

void VScrollArea::childResized(QSize s) {
  //
  updateGeometry();
}

void VScrollArea::scrollTo(int y) {
  if (child)
    child->moveTo(0, -y);
}

void VScrollArea::resizeEvent(QResizeEvent *) {
}


