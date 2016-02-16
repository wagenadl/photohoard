// CropCalc.cpp

#include "CropCalc.h"

CropCalc::CropCalc() {
  setFree();
}

void CropCalc::reset(Adjustments const &a, QSize os) {
  adj = a;
  osize = os;
  setFree();
  fixsize = osize;
  aspect = osize.width() * 1. / osize.height();
  rect = QRectF(cropRect());
}


CropCalc::Orient CropCalc::instantiatedOrientation() const {
  if (orient==Orient::Auto)
    return rect.width()<rect.height() ? Orient::Portrait : Orient::Landscape;
  else
    return orient;
}

double CropCalc::instantiatedAspect() const {
  if (instantiatedOrientation()==Orient::Landscape)
    return aspect;
  else
    return 1./aspect;
}

QSize CropCalc::instantiatedSize() const {
  if (instantiatedOrientation()==Orient::Landscape)
    return fixsize;
  else
    return fixsize.transposed();
}

void CropCalc::setFree() {
  mode = Mode::Free;
  orient = Orient::Auto;
}

double CropCalc::flipIfNeeded(double a) const {
  bool vert0 = rect.height()>rect.height();
  bool vert1 = a<1;
  if (vert0 == vert1)
    return a;
  else
    return 1/a;
}

void CropCalc::setAspect(double a, Orient o) {
  mode = Mode::Aspect;
  if (o==Orient::Auto)
    aspect = flipIfNeeded(a);
  else
    aspect = a;
  if (osize.isNull())
    return;
  
  QPointF center = rect.center();
  double area = rect.width() * rect.height();

  double newwidth = sqrt(area*aspect);
  double newheight = sqrt(area/aspect);
  if (newwidth>osize.width()) {
    newwidth = osize.width();
    newheight = newwidth/a;
  } else if (newheight>osize.height()) {
    newheight = osize.height();
    newwidth = newheight*aspect;
  } // by construction, this limits the w and h to within orig

  setCenterAndSize(center, QSizeF(newwidth, newheight));
}


void CropCalc::setSize(QSize s, Orient o) {
  mode = Mode::Size;
  if (s.width() < s.height())
    s.transpose();
  fixsize = s;
  orient = o;
  if (osize.isNull())
    return;
  
  if (instantiatedOrientation()==Orient::Portrait)
    s.transpose();

  QPointF center = rect.center();

  double newwidth = s.width();
  double newheight = s.height();
  if (newwidth>osize.width()) 
    newwidth = osize.width();
  if (newheight>osize.height()) 
    newheight = osize.height();

  setCenterAndSize(center, QSizeF(newwidth, newheight));
}

void setCenterAndSize(QPointF center, QSizeF s) {
  QPointF tl = center - QPointF(s.width()/2, s.height()/2);
  if (tl.x()<0)
    tl.setX(0);
  else if (tl.x()+s.width() > osize.width())
    tl.setX(osize.width()-s.width());
  if (tl.y()<0)
    tl.setY(0);
  else if (tl.y()+s.height() > osize.height())
    tl.setY(osize.height()-s.height());
  rect = QRectF(tl, QSizeF(s.width(), s.height()));
  updateAdj();
}

void CropCalc::updateAdj() {
  adj.cropl = int(rect.left() + 0.5);
  adj.cropt = int(rect.top() + 0.5);
  adj.cropr = int(osize.width() - rect.right() + 0.5);
  adj.cropb = int(osize.height() - rect.bottom() + 0.5);
}

QRect CropCalc::cropRect() const {
  return QRect(QPoint(adj.cropl, adj.cropt),
               QPoint(osize().width() - adj.cropr,
                      osize().height() - adj.cropb));
}

Adjustments const &CropCalc::adjustments() const {
  return adj;
}

void CropCalc::moveE(double cropr) {
  if (cropr<0)
    cropr = 0;
  double right = osize.width() - cropr;
  if (right - rect.left()<10)
    right = rect.left()+10;
  switch (mode) {
  case Mode::Free:
    rect.setRight(right);
    break;
  case Mode::Aspect: {
    rect.setRight(right);
    double height = rect.width() / instantiatedAspect();
    double deltah2 = (height - rect.height()) / 2;
    if (rect.top() < deltah2)
      rect.setTop(0);
    else
      rect.setTop(rect.top() - deltah2);
    deltah2 = (height - rect.height()) / 2;
    if (osize.height()-rect.bottom() < deltah2)
      rect.setBottom(osize.height());
    else
      rect.setBottom(rect.bottom() + deltah2);
    if (rect.height() != height) {
      double width = instantiatedAspect()*rect.height();
      rect.setLeft(rect.right() - width);
    }
  } break;
  case Mode::Size: {
    rect.setRight(right);
    rect.setLeft(right-instantiatedSize().width());
    if (rect.left()<0)
      rect.setLeft(0);
  } break;
  }
  updateAdj();
}
