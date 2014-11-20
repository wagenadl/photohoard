// SlideView.cpp

#include "SlideView.h"
#include <math.h>
#include <QKeyEvent>
#include <QPainter>
#include <QWheelEvent>
#include <QSize>
#include <QDebug>

SlideView::SlideView(QWidget *parent): QWidget(parent) {
  setObjectName("SlideView");
  fit = true;
}
  
SlideView::~SlideView() {
}

QSize SlideView::desiredSize() const {
  QSize s = (fit || img.isNull()) ? size()
    : QSize(zoom*naturalSize.width(), zoom*naturalSize.height());
  qDebug() << "SlideView::desiredSize: " << s;
  return s;
}

double SlideView::currentZoom() const {
  if (!fit)
    return zoom;
  
  double hrat = width() / double(naturalSize.width());
  double vrat = height() / double(naturalSize.height());
  double rat = hrat<vrat ? hrat : vrat;
  return rat;
}

void SlideView::newImage(QSize nat) {
  qDebug() << "SlideView::newImage " << nat;
  naturalSize = nat;
  img = QImage(); // might invalidate more gently
}  

void SlideView::updateImage(QImage img1) {
  if (img.isNull() || img.width() < img1.width())
    img = img1;
  qDebug() << "SlideView::updateImage " << img1.size();
  update();
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
  QPainter p(this);
  if (fit) {
    QImage i1 = img.scaled(size(), Qt::KeepAspectRatio);
    if (img.width()<naturalSize.width() && i1.width()>img.width())
      emit needLargerImage();
    p.drawImage(QPoint(width()/2 - i1.width()/2, height()/2 - i1.height()/2),
		i1);
  } else {
    double showWidth = zoom*naturalSize.width();
    double showHeight = zoom*naturalSize.height();
    double imgWidth = img.width();
    double imgHeight = img.height();
    double availWidth = width();
    double availHeight = height();
    QRectF sourceRect;
    QRectF destRect;
    if (img.width()<naturalSize.width() && showWidth>img.width())
      emit needLargerImage();
    if (showWidth<=availWidth) {
      sourceRect.setLeft(0);
      sourceRect.setWidth(imgWidth);
      destRect.setLeft(availWidth/2 - showWidth/2);
      destRect.setWidth(showWidth);
    } else {
      destRect.setLeft(0);
      destRect.setWidth(availWidth);
      sourceRect.setWidth(imgWidth * availWidth/showWidth);
      sourceRect.setLeft(relx * (1-availWidth/showWidth) 
			 * imgWidth * availWidth/showWidth);
    }
    if (showHeight<=availHeight) {
      sourceRect.setTop(0);
      sourceRect.setHeight(imgHeight);
      destRect.setTop(availHeight/2 - showHeight/2);
      destRect.setHeight(showHeight);
    } else {
      destRect.setTop(0);
      destRect.setHeight(availHeight);
      sourceRect.setHeight(imgHeight * availHeight/showHeight);
      sourceRect.setTop(rely * (1-availHeight/showHeight) 
			* imgHeight * availHeight/showHeight);
    }
    p.drawImage(destRect, img, sourceRect);
  }
}
    
