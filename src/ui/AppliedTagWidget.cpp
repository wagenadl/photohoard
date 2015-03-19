// AppliedTagWidget.cpp

#include "AppliedTagWidget.h"
#include <QFontMetrics>
#include <QMouseEvent>
#include <QPainter>

AppliedTagWidget::AppliedTagWidget(int id, QString name, QWidget *parent):
  QFrame(parent), id(id), name(name) {
  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  mouseover = false;
  inall = incur = false;
  resize(sizeHint());
}

void AppliedTagWidget::setInclusion(bool inCur, bool inAll) {
  incur = inCur;
  inall = inAll;
  update();
}

QSize AppliedTagWidget::sizeHint() const {
  QFontMetrics fm(font());
  return fm.boundingRect(name).size();
}

QSize AppliedTagWidget::minimumSizeHint() const {
  return sizeHint();
}  

void AppliedTagWidget::enterEvent(QEvent *) {
  mouseover = true;
  update();
}

void AppliedTagWidget::leaveEvent(QEvent *) {
  mouseover = false;
  update();
}

int AppliedTagWidget::buttonSize() const {
  QRect r = contentsRect();
  return r.height() - 6;
}

QRect AppliedTagWidget::removeButtonPlace() const {
  QRect r = contentsRect();
  int s = buttonSize();
  return QRect(QPoint(r.right() - s - 3, r.top() + (r.height()-s)/2),
	       QSize(s, s));
}

QRect AppliedTagWidget::addButtonPlace() const {
  QRect r = contentsRect();
  int s = buttonSize();
  return QRect(QPoint(r.right() - 2*s - 3*2, r.top() + (r.height()-s)/2),
	       QSize(s, s));
}



void AppliedTagWidget::paintEvent(QPaintEvent *) {
  QPainter p(this);
  p.setBrush(QColor("#222222"));
  p.setPen(QPen(Qt::NoPen));
  p.drawRect(contentsRect());
  
  if (inall)
    p.setPen(QColor("#ffffff"));
  else if (incur)
    p.setPen(QColor("#ffff88"));
  else
    p.setPen(QColor("#88ffff"));
  p.drawText(contentsRect(), Qt::AlignLeft | Qt::AlignVCenter,
	     name);
  
  if (mouseover) {
    // draw + and - buttons
    if (!inall) {
      QRect r = addButtonPlace();
      p.setBrush(QBrush("#44ff44"));
      p.setPen(QPen("#000000"));
      p.drawEllipse(r);
      p.drawText(r, Qt::AlignHCenter | Qt::AlignVCenter, "+");
    }
    
    QRect r = addButtonPlace();
    p.setBrush(QBrush("#ff4444"));
    p.setPen(QPen("#000000"));
    p.drawEllipse(r);
    p.drawText(r, Qt::AlignHCenter | Qt::AlignVCenter, QString::fromUtf8("−"));
  }
}

void AppliedTagWidget::mousePressEvent(QMouseEvent *e) {
  if (!inall && addButtonPlace().contains(e->pos()))
    emit addClicked(id);
  else if (removeButtonPlace().contains(e->pos()))
    emit removeClicked(id);
}
