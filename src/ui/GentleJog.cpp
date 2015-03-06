// GentleJog.cpp

#include "GentleJog.h"
#include <QStylePainter>
#include <QDebug>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QTimer>

GentleJog::GentleJog(QString s, QWidget *parent): GentleJog(parent) {
  setLabel(s);
}

GentleJog::GentleJog(QWidget *parent): QFrame(parent) {
  timer = new QTimer(this);
  timer->setSingleShot(true);
  connect(timer, SIGNAL(timeout()), SLOT(timeout()));
  
  min_ = -5;
  max_ = 5;
  dflt_ = 0;
  singlestep = 0.1;
  pagestep = 0.5;
  microstep = 0.01;
  jogdelta = maxdelta = 1;
  val = 0;
  dec = 2;
  jogging = false;
  jogshown = underMouse();
  if (jogshown)
    resetJog();
  lbl = "GentleJog";
  setAutoFillBackground(true);
  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
}

GentleJog::~GentleJog() {
}

QSize GentleJog::minimumSizeHint() const {
  QSize me = size();
  QSize use = contentsRect().size();
  QSize marg = me - use;
  QSize l1 = labelRect().size();
  QSize l2 = valueRect().size();
  QSize s(l1.width() + l2.width() + marg.width(),
          l1.expandedTo(l2).height() + marg.height());
  return s;
}

QSize GentleJog::sizeHint() const {
  return minimumSizeHint();
}

void GentleJog::setLabel(QString s) {
  lbl = s;
  update();
}

void GentleJog::setDefault(double m) {
  dflt_ = m;
}

void GentleJog::setMinimum(double m) {
  min_ = m;
  if (clamp())
    update();
}

void GentleJog::setMaximum(double m) {
  max_ = m;
  if (clamp())
    update();
}

void GentleJog::setRange(double m, double m1) {
  min_ = m;
  max_ = m1;
  if (clamp())
    update();
}

void GentleJog::setDecimals(int n) {
  dec = n;
  update();
}

void GentleJog::setSingleStep(double s) {
  singlestep = s;
}

void GentleJog::setPageStep(double s) {
  pagestep = s;
}

void GentleJog::setMicroStep(double s) {
  microstep = s;
}

void GentleJog::setSteps(double s, double p, double m) {
  singlestep = s;
  pagestep = p>0 ? p : s;
  microstep = m>0 ? m : s;
}

void GentleJog::setMaxDelta(double d) {
  maxdelta = d;
}

void GentleJog::setValueQuietly(double f) {
  double v0 = val;
  val = f;
  clamp();
  if (v0 != val) 
    update();
}

void GentleJog::setValue(double f) {
  double v0 = val;
  setValueQuietly(f);
  if (val != v0)
    emit valueChanged(val);
}

bool GentleJog::clamp() {
  if (val>=min_ && val<=max_)
    return false;
  
  if (val>=max_)
    val = max_;
  else
    val = min_;
  return true;
}

QRect GentleJog::labelRect() const {
  QFontMetrics fm(font());
  QRect r0 = contentsRect();
  QRect r1 = fm.boundingRect(lbl);
  r1.moveTo(r0.left(), r0.top() + (r0.height()-r1.height())/2);
  return r1;
}

QRect GentleJog::valueRect() const {
  QFontMetrics fm(font());
  return fm.boundingRect(contentsRect(),
			 Qt::AlignRight | Qt::AlignVCenter,
			 valueText());
}

QString GentleJog::valueText() const {
  QString zwnj = QString::fromUtf8("‌"); // zero width non-joiner 0x200c
  double thr = .5*pow(10, -dec);
  QString sgn = (val>thr && min_<0) ? QString::fromUtf8("+")
    : (val<-thr) ? QString::fromUtf8("−")
    : QString::fromUtf8(" ");
  return zwnj + sgn + QString::number(fabs(val), 'f', dec) + zwnj;
}

void GentleJog::renderLabel(QStylePainter *p) {
  QPen pen(p->pen());
  pen.setColor(palette().color(jogshown ? QPalette::Disabled
                               : QPalette::Normal,
                               QPalette::WindowText));
  p->setPen(pen);
  p->drawText(contentsRect(),
	      Qt::AlignLeft | Qt::AlignVCenter,
	      lbl);
}

void GentleJog::renderValue(QStylePainter *p) {
  QPen pen(p->pen());
  pen.setColor(palette().color(fabs(val-dflt_) < .5*pow(10, -dec)
                               ? QPalette::Disabled : QPalette::Normal,
                               QPalette::WindowText));
  p->setPen(pen);
    
  p->drawText(contentsRect(),
	      Qt::AlignRight | Qt::AlignVCenter,
	      valueText());
}

void GentleJog::renderJog(QStylePainter *p) {
  QRect r0 = contentsRect();
  int y0 = r0.center().y();
  QPen pen(p->pen());
  QColor c0(palette().color(QPalette::Normal, QPalette::BrightText)); // white
  QColor c1(palette().color(QPalette::Normal, QPalette::WindowText)); // black
  QColor c(c0);
  c.setAlpha(96);
  pen.setColor(c);
  pen.setWidth(5);
  p->setPen(pen);
  p->drawLine(jogminx - 2, y0, jogmaxx + 2, y0);
  pen.setWidth(3);
  p->setPen(pen);
  p->drawLine(jogminx - 2, y0, jogmaxx + 2, y0);

  pen.setColor(c0);
  pen.setWidth(1);
  p->setPen(pen);
  p->drawLine(jogminx - 1, y0+1, jogmaxx + 3, y0+1);
  pen.setColor(c1);
  pen.setWidth(1);
  p->setPen(pen);
  p->drawLine(jogminx - 2, y0, jogmaxx + 2, y0);
  
  int x = jogx;
  if (x<jogminx)
    x = jogminx;
  else if (x>jogmaxx)
    x = jogmaxx;

  pen.setColor(c);
  pen.setWidth(2);
  p->setPen(pen);
  p->drawEllipse(QPoint(x, y0), jogr, jogr);
  QBrush b(palette().color(QPalette::Normal, QPalette::WindowText));
  p->setBrush(b);
  pen.setColor(c0);
  pen.setWidth(1);
  p->setPen(pen);
  p->drawEllipse(QPoint(x, y0), jogr, jogr);
}

void GentleJog::resetJog() {
  QRect r0 = contentsRect();
  jogr = r0.height()/3;
  jogminx = r0.left() + jogr;
  jogmaxx = valueRect().left() - 2 - jogr;
  jogx = deltaToX(0);
}

void GentleJog::paintEvent(QPaintEvent *) {
  QStylePainter p(this);
  renderValue(&p);
  renderLabel(&p);
  if (jogshown)
    renderJog(&p);
}
  
void GentleJog::mousePressEvent(QMouseEvent *e) {
  if (e->button() == Qt::LeftButton)
    startJog(e->x(), e->modifiers());  
}

void GentleJog::mouseReleaseEvent(QMouseEvent *e) {
  if (e->button() == Qt::LeftButton)
    endJog(e->x());
}

void GentleJog::mouseMoveEvent(QMouseEvent *e) {
  if (jogging)
    continueJog(e->x());
}

void GentleJog::mouseDoubleClickEvent(QMouseEvent */* e */) {
  //  if (valueRect().contains(e->pos())) 
    setValue(dflt_);
}

double GentleJog::stepFor(Qt::KeyboardModifiers m) {
  return (m & Qt::ShiftModifier) ? pagestep
    : (m & (Qt::ControlModifier|Qt::AltModifier)) ? microstep
    : singlestep;
}

void GentleJog::wheelEvent(QWheelEvent *e) {
  setValueVisually(val + e->delta()*stepFor(e->modifiers())/120);
  timer->start(300);
}

void GentleJog::keyPressEvent(QKeyEvent *e) {
  switch (e->key()) {
  case Qt::Key_Left: case Qt::Key_Down:
    setValueVisually(val - stepFor(e->modifiers()));
    break;
  case Qt::Key_Right: case Qt::Key_Up:
    setValueVisually(val + stepFor(e->modifiers()));
    break;
  case Qt::Key_PageDown:
    setValueVisually(val - pagestep);
    break;
  case Qt::Key_PageUp:
    setValueVisually(val + pagestep);
    break;
  default:
    break;
  }    
}

void GentleJog::keyReleaseEvent(QKeyEvent *e) {
  switch (e->key()) {
  case Qt::Key_Left: case Qt::Key_Down:
    break;
  case Qt::Key_Right: case Qt::Key_Up:
    break;
  case Qt::Key_PageDown:
    break;
  case Qt::Key_PageUp:
    break;
  default:
    return;
  }    
  if (jogging)
    timer->start(150);
}

void GentleJog::enterEvent(QEvent *) {
  setFocus(Qt::MouseFocusReason);
}

void GentleJog::focusInEvent(QFocusEvent *) {
  jogshown = true;
  resetJog();
  update();
}

void GentleJog::resizeEvent(QResizeEvent *) {
  if (jogshown)
    resetJog();
  update();
}

void GentleJog::leaveEvent(QEvent *) {
  //  clearFocus();
}

void GentleJog::focusOutEvent(QFocusEvent *) {
  jogshown = false;
  update();
}

double GentleJog::maprev(double x) {
  return tan(x)/tan(1);
}

double GentleJog::mapfwd(double x) {
  return atan(x*tan(1));
}

int GentleJog::deltaToX(double dv) {
  return (mapfwd(dv/jogdelta) + 1.0) * (jogmaxx - jogminx)/2.0 + jogminx;
}

double GentleJog::xToDelta(int x) {
  if (x<jogminx)
    return -jogdelta;
  else if (x>jogmaxx)
    return jogdelta;
  else
    return maprev((x-jogminx)/((jogmaxx-jogminx)/2.0) - 1.0) * jogdelta;
}

void GentleJog::startJog(int x, Qt::KeyboardModifiers m) {
  timer->stop();
  if (x>=jogminx && x<=jogmaxx) {
    jogging = true;
    jogdx0 = 0;
    jogv0 = val;
    jogdelta = maxdelta;
    if (m & Qt::ShiftModifier)
      jogdelta *= 5;
    else if (m & (Qt::ControlModifier | Qt::AltModifier))
      jogdelta /= 5;
    continueJog(x);
  }
}

void GentleJog::continueJog(int x) {
  if (!jogging)
    return;
  jogx = x - jogdx0;
  setValue(jogv0 + xToDelta(x-jogdx0));
}

void GentleJog::endJog(int x) {
  if (!jogging)
    return;
  setValue(jogv0 + xToDelta(x-jogdx0));
  timer->start(100);
}

void GentleJog::setValueVisually(double v) {
  double v0 = val;
  setValue(v);
  if (!jogshown)
    return;
  if (!jogging) {
    resetJog();
    jogv0 = v0;
  }
  jogx = deltaToX(val - jogv0);
  jogging = true;
  update();
}

void GentleJog::timeout() {
  resetJog();
  jogging = false;
  update();
}
