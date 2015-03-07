// SlideView.cpp

#include "SlideView.h"
#include <math.h>
#include <QKeyEvent>
#include <QPainter>
#include <QWheelEvent>
#include "PSize.h"
#include <QDebug>
#include "CMS.h"

SlideView::SlideView(QWidget *parent): QFrame(parent) {
  setObjectName("SlideView");
  fit = true;
}
  
SlideView::~SlideView() {
}

PSize SlideView::desiredSize() const {
  PSize s = (fit || img.isNull()) ? size()
    : PSize(zoom*naturalSize.width(), zoom*naturalSize.height());
  return s;
}

double SlideView::currentZoom() const {
  if (!fit)
    return zoom;
  else
    return fittingZoom();
}

double SlideView::fittingZoom() const {  
  double hrat = width() / double(naturalSize.width());
  double vrat = height() / double(naturalSize.height());
  double rat = hrat<vrat ? hrat : vrat;
  return rat;
}

void SlideView::newImage(PSize nat) {
  naturalSize = nat;
  lastSize = PSize();
  img = Image16(); // might invalidate more gently
}  

void SlideView::updateImage(Image16 img1) {
  if (img.isNull() || img.width() < img1.width()) {
    if (CMS::monitorTransform.isValid()) {
      img = CMS::monitorTransform.apply(img1);
      qDebug() << "Applied monitor transform";
    } else {
      img = img1;
    }
  }
  qDebug() << "  Requesting update";
  update();
  qDebug() << "  Request posted";
}

void SlideView::changeZoomLevel(QPoint, double delta) {
  if (fit) {
    relx = .5;
    rely = .5;
    zoom = currentZoom();
    fit = false;
  }
  zoom *= pow(2, delta);
  update();
}

void SlideView::setZoom(double z) {
  if (fit) {
    relx = .5;
    rely = .5;
  }
  fit = false;
  zoom = z;
  update();
}
  
void SlideView::scaleToFit() {
  fit = true;
  update();
}

void SlideView::resizeEvent(QResizeEvent *) {
  emit newSize(size());
  if (fit)
    update();
}

void SlideView::wheelEvent(QWheelEvent *e) {
  qDebug() << "SlideView::wheelEvent " << e->delta();
  changeZoomLevel(e->pos(), e->delta()/1000.0);
}

void SlideView::keyPressEvent(QKeyEvent *e) {
  qDebug() << "SlideView::keyEvent " << e->key();
  switch (e->key()) {
  case Qt::Key_0:
    scaleToFit();
    break;
  case Qt::Key_Minus:
    changeZoomLevel(QPoint(), -0.5);
    break;
  case Qt::Key_Equal: case Qt::Key_Plus:
    changeZoomLevel(QPoint(), 0.5);
    break;
  case Qt::Key_1:
    setZoom(1);
    break;
  }
}

void SlideView::mousePressEvent(QMouseEvent *) {
  // start drag
}

void SlideView::mouseMoveEvent(QMouseEvent *) {
  // move image around
}

void SlideView::mouseDoubleClickEvent(QMouseEvent *) {
  emit doubleClicked();
}

void SlideView::paintEvent(QPaintEvent *) {
  qDebug() << "SlideView::paint";
  if (img.isNull())
    return;
  
  QPainter p(this);
  QRect r = contentsRect();
  
  if (fit) {
    qDebug() << "  scaling";
    Image16 i1 = img.scaled(r.size(), Qt::KeepAspectRatio);
    qDebug() << "  scaled";
    if (img.width()<naturalSize.width()
        && i1.width()>img.width()
        && img.height()<naturalSize.height()
        && i1.height()>img.height()) {
      // I should only request it if I haven't already
      if (img.size()!=lastSize) {
        qDebug() << "  needlargerimage";
        emit needLargerImage();
      }
      lastSize = img.size();
    } else {
      lastSize = PSize();
    }
    qDebug() << "  drawing image";
    p.drawImage(QPoint((r.left() + r.right())/2 - i1.width()/2,
                       (r.top() + r.bottom())/2 - i1.height()/2),
		i1.toQImage());
  } else {
    qDebug() << "  not fit";
    PSize showSize = naturalSize*zoom;
    PSize availSize = r.size();
    QRectF sourceRect;
    QRectF destRect;
    if (!img.size().isLargeEnoughFor(showSize)
	&& naturalSize.exceeds(img.size())) {
      if (img.size()!=lastSize) 
	emit needLargerImage();
      lastSize = img.size();
    } else {
      lastSize = PSize();
    }
    if (showSize.width()<=availSize.width()) {
      sourceRect.setLeft(0);
      sourceRect.setWidth(img.width());
      destRect.setLeft((r.left()+r.right())/2 - showSize.width()/2);
      destRect.setWidth(showSize.width());
    } else {
      destRect.setLeft(r.left());
      destRect.setWidth(availSize.width());
      sourceRect.setWidth(img.width()*availSize.width()/showSize.width());
      sourceRect.setLeft(relx * (1-(0.+availSize.width())/showSize.width()) 
			 * img.width() * availSize.width()/showSize.width());
    }
    if (showSize.height()<=availSize.height()) {
      sourceRect.setTop(r.top());
      sourceRect.setHeight(img.height());
      destRect.setTop((r.top()+r.bottom())/2 - showSize.height()/2);
      destRect.setHeight(showSize.height());
    } else {
      destRect.setTop(0);
      destRect.setHeight(availSize.height());
      sourceRect.setHeight(img.height() * availSize.height()/showSize.height());
      sourceRect.setTop(rely * (1-(0.+availSize.height())/showSize.height()) 
			* img.height() * availSize.height()/showSize.height());
    }
    p.drawImage(destRect, img.toQImage(), sourceRect);
  }
}
    
