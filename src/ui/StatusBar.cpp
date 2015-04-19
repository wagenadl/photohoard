// StatusBar.cpp

#include "StatusBar.h"
#include <QPainter>
#include "PDebug.h"

constexpr int SB_Height = 20;

StatusBar::StatusBar(QWidget *parent): QFrame(parent) {
  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
  zoom = 0;
  setContentsMargins(3, 0, 3, 0);
}

QSize StatusBar::sizeHint() const {
  return QSize(200, SB_Height);
}

QSize StatusBar::minimumSizeHint() const {
  return QSize(40, SB_Height);
}

void StatusBar::setZoom(double v) {
  pDebug() << "StatusBar::setZoom" << v;
  zoom = v;
  update();
}

void StatusBar::setCollection(QString c) {
  col = c;
  update();
}

void StatusBar::paintEvent(QPaintEvent *) {
  QRect r = contentsRect();
  QPainter p(this);
  p.setBrush(QColor("#eeeeee"));
  p.setPen(QPen(Qt::NoPen));
  p.drawRect(r);
  p.setPen(QPen("#000000"));
  if (zoom>0 && zoom<1e5)
    p.drawText(r, Qt::AlignVCenter | Qt::AlignRight,
               QString("%1%").arg(round(zoom*100)));
  if (!col.isEmpty())
    p.drawText(r, Qt::AlignVCenter | Qt::AlignLeft, col);
}
