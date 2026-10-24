// AppliedTagWidget.cpp

#include "AppliedTagWidget.h"
#include <QFontMetrics>
#include <QMouseEvent>
#include <QPainter>
#include "PDebug.h"
#include "Style.h"

AppliedTagWidget::AppliedTagWidget(int id, QString name, bool ro,
				   QWidget *parent):
  QFrame(parent), id(id), name(name), ro(ro) {
  //  setStyleSheet("QWidget { background-color: '#aaaaaa'; color: '#000000'; }");
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

void AppliedTagWidget::enterEvent(QEnterEvent *) {
  if (ro)
    return;
  mouseover = true;
  update();
}

void AppliedTagWidget::leaveEvent(QEvent *) {
  if (ro)
    return;
  mouseover = false;
  update();
}

int AppliedTagWidget::buttonSize() const {
  QRect r = contentsRect();
  return r.height() - 4;
}

QRect AppliedTagWidget::removeButtonPlace() const {
  QRect r = contentsRect();
  int s = buttonSize();
  return QRect(QPoint(r.right() - s, r.top() + (r.height()-s)/2),
	       QSize(s, s));
}

QRect AppliedTagWidget::addButtonPlace() const {
  QRect r = contentsRect();
  int s = buttonSize();
  return QRect(QPoint(r.right() - 2*s - 2, r.top() + (r.height()-s)/2),
	       QSize(s, s));
}



void AppliedTagWidget::paintEvent(QPaintEvent *) {
  bool isdark = Style::instance()->isDark();
  QPainter p(this);
  
  if (inall)
    p.setPen(isdark ? QColor("#ffffff") : QColor("#000000"));
  //else if (incur)
  //    p.setPen(QColor("#883300"));
  else
    p.setPen(QColor("#4466ee"));
  p.drawText(contentsRect(), Qt::AlignLeft | Qt::AlignVCenter,
	     name);
  
  if (mouseover) {
    // draw + and - buttons
    if (!inall) {
      QRect r = addButtonPlace();
      p.setBrush(isdark ? QBrush("#008800") : QBrush("#66ff66"));
      p.setPen(isdark ? QColor("#000000") : QColor("#ffffff"));
      p.drawEllipse(r);
      p.setPen(isdark ? QColor("#ffffff") : QColor("#000000"));
      p.drawText(r, Qt::AlignHCenter | Qt::AlignVCenter, "+");
    }
    
    QRect r = removeButtonPlace();
    QColor rmc = isdark ? QColor("#880000") : QColor("#ff6666");
    p.setPen(isdark ? QColor("#000000") : QColor("#ffffff"));
    p.setBrush(rmc);
    p.drawEllipse(r);
    p.setPen(isdark ? QColor("#ffffff") : QColor("#000000"));
    p.drawText(r, Qt::AlignHCenter | Qt::AlignVCenter, QString::fromUtf8("−"));
  }
  if (!p.isActive())
    COMPLAIN("Painter not active in AppliedTagWidget.cpp");
}

void AppliedTagWidget::mousePressEvent(QMouseEvent *e) {
  if (ro)
    return;
  if (!inall && addButtonPlace().contains(e->pos()))
    emit addClicked(id);
  else if (removeButtonPlace().contains(e->pos()))
    emit removeClicked(id);
}
