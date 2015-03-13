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
  zoom = 1;
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
  if (rat>1)
    rat = 1;
  return rat;
}

void SlideView::newImage(QSize nat) {
  naturalSize = nat;
  lastSize = PSize();
  img = Image16(); // might invalidate more gently
  fit = true;
  zoom = 1;
}  

void SlideView::updateImage(Image16 img1, bool force) {
  if (force || img.isNull() || img.width() < img1.width()) {
    if (CMS::monitorTransform.isValid()) {
      img = CMS::monitorTransform.apply(img1);
    } else {
      img = img1;
    }
  }
  update();
}

void SlideView::changeZoomLevel(QPoint, double delta) {
  setZoom(currentZoom() * pow(2.0, delta));
}

void SlideView::setZoom(double z) {
  if (z<fittingZoom()) {
    if (!fit)
      scaleToFit();
  } else {
    if (fit) {
      fit = false;
      relx = rely = 0.5;
    }
    zoom = z;
    emit newSize(naturalSize.isEmpty() ? size() : naturalSize*zoom);
    update();
  }xxx
}
  
void SlideView::scaleToFit() {
  fit = true;
  relx = rely = 0.5;
  emit newSize(size());
  update();
}

void SlideView::resizeEvent(QResizeEvent *) {
  emit newSize(fit ? size() : naturalSize.isEmpty() ? size()
	       : naturalSize*zoom);
  if (fit)
    update();
}

void SlideView::wheelEvent(QWheelEvent *e) {
  changeZoomLevel(e->pos(), e->delta()/1000.0);
}

void SlideView::keyPressEvent(QKeyEvent *e) {
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

void SlideView::mousePressEvent(QMouseEvent *e) {
  if (e->button()==Qt::LeftButton && !fit) {
    presspoint = e->pos();
    pressrelx = relx;
    pressrely = rely;
    dragging = true;
    e->accept();
  } else {
    dragging = false;
  }
}

void SlideView::mouseMoveEvent(QMouseEvent *e) {
  if (dragging) 
    updateRelXY(e->pos());
}

void SlideView::mouseDoubleClickEvent(QMouseEvent *) {
  emit doubleClicked();
}

void SlideView::updateRelXY(QPoint p) {
  QPoint dp = p - presspoint;
  QRect r = contentsRect();
  PSize availSize = r.size();
  relx = pressrelx - dp.x()*2./availSize.width();
  rely = pressrely - dp.y()*2./availSize.height();
  if (relx<0)
    relx = 0;
  else if (relx>1)
    relx = 1;
  if (rely<0)
    rely = 0;
  else if (rely>1)
    rely = 1;
  pressrelx = relx;
  pressrely = rely;
  presspoint = p;
  update();
}    

void SlideView::paintEvent(QPaintEvent *) {
  if (img.isNull())
    return;
  
  QPainter p(this);
  QRect r = contentsRect();
  
  if (fit) {
    qDebug() << "SlideView::paintEvent av=" << img.size()
	     << "space=" << r.size()
	     << "nat=" << naturalSize;
    Image16 i1 = img.scaledToFitIn(r.size());
    if (img.width()<naturalSize.width()
        && i1.width()>img.width()
        && img.height()<naturalSize.height()
        && i1.height()>img.height()) {
      // I should only request it if I haven't already
      if (img.size()!=lastSize) {
        emit needLargerImage();
      }
      lastSize = img.size();
    } else {
      lastSize = PSize();
    }
    p.drawImage(QPoint((r.left() + r.right())/2 - i1.width()/2,
                       (r.top() + r.bottom())/2 - i1.height()/2),
		i1.toQImage());
  } else {
    PSize showSize = naturalSize*zoom;
    PSize availSize = r.size();
    double effZoom = img.size().scaleFactorToFitIn(showSize);
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
      destRect.setWidth(img.width()*effZoom);
    } else {
      destRect.setLeft(r.left());
      destRect.setWidth(availSize.width());
      sourceRect.setLeft(relx * (1-availSize.width()/double(showSize.width()))
			 * img.width());
      sourceRect.setWidth(availSize.width()/effZoom);
    }
    if (showSize.height()<=availSize.height()) {
      sourceRect.setTop(r.top());
      sourceRect.setHeight(img.height());
      destRect.setTop((r.top()+r.bottom())/2 - showSize.height()/2);
      destRect.setHeight(img.height()*effZoom);
    } else {
      destRect.setTop(0);
      destRect.setHeight(availSize.height());
      sourceRect.setTop(rely * (1-availSize.height()/double(showSize.height()))
			* img.height());
      sourceRect.setHeight(availSize.height()/effZoom);
    }
    p.drawImage(destRect, img.toQImage(), sourceRect);
  }
}
    
