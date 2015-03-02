// ControlGroup.cpp

#include "ControlGroup.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QPainter>

ControlGroup::ControlGroup(QString l, QWidget *p): QFrame(p) {
  my_lay = new QVBoxLayout(this);
  my_lay->setContentsMargins(7, 0, 7, 0);

  exp_w = new QWidget();
  exp_lay = new QVBoxLayout(exp_w);
  exp_lay->setContentsMargins(0, 1, 0, 1);
  my_lay->addWidget(exp_w);

  col_w = new QWidget();
  col_lay = new QHBoxLayout(col_w);
  col_lay->setContentsMargins(0, 0, 0, 0);
  my_lay->addWidget(col_w);

  hdr = new QLabel();
  col_lay->addStretch(1);
  col_lay->addWidget(hdr);
  col_lay->addStretch(1);
  //  expander = new QRadioButton("");
  //  expander->setFocusPolicy(Qt::NoFocus);
  //  col_lay->addWidget(expander);

  setLabel(l);
  collapse();

  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
  //  connect(expander, SIGNAL(clicked()), SLOT(expand()));
}

ControlGroup::~ControlGroup() {
}

void ControlGroup::setLabel(QString l) {
  hdr->setText(l);
}

void ControlGroup::expand() {
  //  expander->setChecked(false);
  exp_w->show();
  col_w->hide();
}

void ControlGroup::collapse() {
  exp_w->hide();
  col_w->show();
}

void ControlGroup::paintEvent(QPaintEvent *) {
  QPainter p(this);
  QPen pen(p.pen());
  pen.setWidth(2);
  pen.setJoinStyle(Qt::MiterJoin);
  p.setPen(pen);
  QRect r = contentsRect();
  QPolygon poly;
  poly << QPoint(r.left()+6, r.top());
  poly << QPoint(r.left(), r.top());
  poly << QPoint(r.left(), r.bottom()-1);
  poly << QPoint(r.left()+6, r.bottom()-1);
  p.drawPolyline(poly);

  poly.clear();
  poly << QPoint(r.right()-1-6, r.top());
  poly << QPoint(r.right()-1-0, r.top());
  poly << QPoint(r.right()-1-0, r.bottom()-1);
  poly << QPoint(r.right()-1-6, r.bottom()-1);
  p.drawPolyline(poly);  
}

void ControlGroup::mousePressEvent(QMouseEvent *) {
  if (isExpanded())
    collapse();
  else
    expand();
}

void ControlGroup::resizeEvent(QResizeEvent *) {
  update();
}

void ControlGroup::keyPressEvent(QKeyEvent *) {
}

void ControlGroup::addWidget(QWidget *w) {
  exp_lay->addWidget(w);
}

void ControlGroup::addLayout(QLayout *w) {
  exp_lay->addLayout(w);
}
