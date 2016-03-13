// Tristate.cpp

#include "Tristate.h"
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QMouseEvent>
#include "PDebug.h"

Tristate::Tristate(QWidget *parent): Tristate("", parent) {
  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}

Tristate::Tristate(QString txt, QWidget *parent): QWidget(parent), txt(txt) {
  s = Off;
}

Tristate::~Tristate() {
}

void Tristate::setState(State s1) {
  if (s1==s)
    return;
  s = s1;
  update();
  emit stateChanged(s);
}

void Tristate::setText(QString t) {
  txt = t;
  update();
}

void Tristate::mousePressEvent(QMouseEvent *e) {
  if (e->button()==Qt::LeftButton) {
    if (s==On)
      s = Off;
    else
      s = On;
    emit clicked();
    emit toggled(s==On ? true : false);
    emit stateChanged(s);
    update();
    e->accept();
  } else {
    e->ignore();
  }
}

QSize Tristate::sizeHint() const {
  QFontMetrics fm(font());
  QRect rtxt = fm.boundingRect(txt);
  QRect rx = fm.tightBoundingRect("X");
  rtxt |= rx;
  double wx = rx.height() + 2;
  double dx = rx.width() / 2.;
  return QSize(wx + dx + 2, 0) + rtxt.size();
}

QSize Tristate::minimumSizeHint() const {
  return sizeHint();
}

void Tristate::paintEvent(QPaintEvent *) {
  QPainter ptr(this);
  QFontMetrics fm(font());
  //  QRect rtxt = fm.boundingRect(txt);
  QRect rx = fm.tightBoundingRect("X");
  double wx = rx.height() + 2;
  double dx = rx.width() / 2.;
  double xtxt = wx + dx;
  double y0 = height()/2.0 - wx/2;

  QPen pen(QColor(128,128,128));
  ptr.setPen(pen);
  QRectF box(QPointF(0, y0-1), QSize(wx, wx));
  ptr.drawRect(box);

  pDebug() << "tristate" << this << "paint" << int(s);
  
  box.adjust(2, 2, -1, -1);
  ptr.setPen(Qt::NoPen);
  ptr.setBrush(QBrush(QColor(0, 0, 0)));
  switch (s) {
  case Off:
    break;
  case Undef:
    ptr.drawPolygon(QPolygonF()
                    << box.bottomLeft() + QPointF(0, -1)
                    << box.bottomRight() 
                    << box.topRight() + QPointF(-1, 0));
    break;
  case On:
    ptr.drawRect(box);
    break;
  }

  pen.setColor(QColor(0, 0, 0));
  ptr.setPen(pen);
  ptr.drawText(QRectF(QPointF(xtxt, 0), QSizeF(width()-xtxt, height())),
                Qt::AlignLeft | Qt::AlignVCenter,
                txt);
}

