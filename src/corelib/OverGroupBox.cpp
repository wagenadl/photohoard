// OverGroupBox.cpp

#include "OverGroupBox.h"
#include <QDebug>
#include <QMouseEvent>
#include <QApplication>

class OGB_Overlay: public QWidget {
public:
  OGB_Overlay(OverGroupBox *parent): QWidget(parent), parent_(parent) {
    setAttribute(Qt::WA_TranslucentBackground);
  }
  virtual void mousePressEvent(QMouseEvent *e) {
    parent_->overlayPressEvent(e);
    e->accept();
  }
  virtual void mouseReleaseEvent(QMouseEvent *e) {
    parent_->overlayReleaseEvent(e);
  }
private:
  OverGroupBox *parent_;
};
    

OverGroupBox::OverGroupBox(QWidget *parent): QGroupBox(parent) {
  createOverlay();
  pt = false;
}

OverGroupBox::OverGroupBox(QString title, QWidget *parent):
  QGroupBox(title, parent) {
  createOverlay();
  pt = false;
}

void OverGroupBox::createOverlay() {
  overlay = new OGB_Overlay(this);
  overlay->resize(size());
  connect(this, SIGNAL(toggled(bool)),
	  this, SLOT(respondToToggle(bool)));
  if (!isCheckable() || isChecked())
    overlay->hide();
}

void OverGroupBox::respondToToggle(bool b) {
  overlay->setVisible(!b);
  if (!b) {
    overlay->setEnabled(true);
    overlay->raise();
  }
}

void OverGroupBox::resizeEvent(QResizeEvent *e) {
  overlay->resize(size());
  overlay->raise();
  QGroupBox::resizeEvent(e);
}

void OverGroupBox::overlayPressEvent(QMouseEvent *) {
}

void OverGroupBox::overlayReleaseEvent(QMouseEvent *e) {
  setChecked(true);
  overlay->hide();
  if (pt) {
    QWidget *ch = childAt(e->pos());
    if (ch) {
      QMouseEvent *e1 = new QMouseEvent(QEvent::MouseButtonPress,
					ch->mapFromParent(e->pos()),
					e->button(),
					e->buttons(),
					e->modifiers());
      QApplication::postEvent(ch, e1);
      QMouseEvent *e2 = new QMouseEvent(QEvent::MouseButtonRelease,
					ch->mapFromParent(e->pos()),
					e->button(),
					e->buttons(),
					e->modifiers());
      QApplication::postEvent(ch, e2);
    }
  }
}

void OverGroupBox::setPassThroughEnabled(bool b) {
  pt = b;
}

