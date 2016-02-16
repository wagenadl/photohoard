// CropCalc.cpp

#include "CropCalc.h"

inline double clip(double v, double minv, double maxv) {
  if (v<minv)
    return minv;
  else if (v>maxv)
    return maxv;
  else
    return v;
}

CropCalc::CropCalc() {
  setFree();
  dx = dy = 0.7;
}

void CropCalc::reset(Adjustments const &a, QSize os) {
  adj = a;
  osize = os;
  setFree();
  aspect = osize.width() * 1. / osize.height();
  calcDxy();
  rect = QRectF(cropRect());
}

void CropCalc::setFree() {
  mode = Mode::Free;
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

  calcDxy();

  if (osize.isNull())
    return;
  
  QPointF center = rect.center();
  double area = rect.width() * rect.height();

  double newwidth = sqrt(area*aspect);
  double newheight = sqrt(area/aspect);
  if (newwidth>osize.width()) {
    newwidth = osize.width();
    newheight = newwidth/a;
    // By construction, newheight<=osize.height.
  } else if (newheight>osize.height()) {
    newheight = osize.height();
    newwidth = newheight*aspect;
    // By construction, newwidth<=osize.width.
  }

  // Ideally, place centered on previous crop rectangle: ...
  QPointF tl = center - QPointF(newwidth/2, newheight/2);
  // ... But be prepared to shift around:
  if (tl.x()<0)
    tl.setX(0);
  else if (tl.x()+newwidth > osize.width())
    tl.setX(osize.width()-newwidth);
  if (tl.y()<0)
    tl.setY(0);
  else if (tl.y()+newheight > osize.height())
    tl.setY(osize.height()-newheight);
  
  rect = QRectF(tl, QSizeF(newwidth, newheight));

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

void CropCalc::calcDxy() {
  double dy2 = 1/(1+aspect*aspect);
  dy = sqrt(dy2);
  dx = sqrt(1-dy2);
}  

//////////////////////////////////////////////////////////////////////
double CropCalc::pseudoSliderValueTL() const {
  return clip(adj.cropl/dx, 0, adj.cropt/dy);
}

double CropCalc::pseudoSliderValueTR() const {
  return clip(adj.cropr/dx, 0, adj.cropt/dy);
}

double CropCalc::pseudoSliderValueBL() const {
  return clip(adj.cropl/dx, 0, adj.cropb/dy);
}

double CropCalc::pseudoSliderValueBR() const {
  return clip(adj.cropr/dx, 0, adj.cropb/dy);
}

//////////////////////////////////////////////////////////////////////
// I think the following are correc. Not quite sure.
double CropCalc::pseudoSliderMaxTL() const {
  return clip(pseudoSliderMaxLeft()/dx, 0, pseudoSliderMaxTop()/dy);
}

double CropCalc::pseudoSliderMaxTR() const {
  return clip(pseudoSliderMaxRight()/dx, 0, pseudoSliderMaxTop()/dy);
}

double CropCalc::pseudoSliderMaxBL() const {
  return clip(pseudoSliderMaxLeft()/dx, 0, pseudoSliderMaxBottom()/dy);
}

double CropCalc::pseudoSliderMaxBR() const {
  return clip(pseudoSliderMaxRight()/dx, 0, pseudoSliderMaxBottom()/dy);
}

//////////////////////////////////////////////////////////////////////
double CropCalc::pseudoSliderMaxLeft() const {
  return rect.right() - 10;
}

double CropCalc::pseudoSliderMaxRight() const {
  return osize.width() - rect.left() - 10;
}

double CropCalc::pseudoSliderMaxTop() const {
  return rect.bottom() - 10;
}

double CropCalc::pseudoSliderMaxBottom() const {
  return osize.height() - rect.top() - 10;
}

//////////////////////////////////////////////////////////////////////
void CropCalc::slideLeft(double cropl) {
  cropl = clip(cropl, 0, pseudoSliderMaxLeft());
  rect.setLeft(cropl);
  if (mode==Mode::Aspect) {
    double wid = rect.width();
    double hei = wid/aspect;
    expandTop((hei - rect.height())/2);
    expandBottom(hei - rect.height());
    expandTop((hei - rect.height()));
    rect.setRight(rect.left() + rect.height()*aspect);
  }
  updateAdj();
}

void CropCalc::slideRight(double cropr) {
  cropr = clip(cropr, 0, pseudoSliderMaxRight());
  rect.setRight(osize.width() - cropr);
  if (mode==Mode::Aspect) {
    double wid = rect.width();
    double hei = wid/aspect;
    expandTop((hei - rect.height())/2);
    expandBottom(hei - rect.height());
    expandTop((hei - rect.height()));
    rect.setLeft(rect.right() - rect.height()*aspect);
  }
  updateAdj();
}

void CropCalc::slideTop(double cropt) {
  cropt = clip(cropt, 0, pseudoSliderMaxTop());
  rect.setTop(cropt);
  if (mode==Mode::Aspect) {
    double hei = rect.height();
    double wid = hei*aspect;
    expandLeft((wid - rect.width())/2);
    expandRight(wid - rect.width());
    expandLeft((wid - rect.width()));
    rect.setBottom(rect.top() + rect.width()/aspect);
  }
  updateAdj();
}

void CropCalc::slideBottom(double cropb) {
  cropb = clip(cropb, 0, pseudoSliderMaxBottom());
  rect.setBottom(osize.height() - cropb));
  if (mode==Mode::Aspect) {
    double hei = rect.height();
    double wid = hei*aspect;
    expandLeft((wid - rect.width())/2);
    expandRight(wid - rect.width());
    expandLeft((wid - rect.width()));
    rect.setTop(rect.bottom() - rect.width()/aspect);
  }
  updateAdj();
}

//////////////////////////////////////////////////////////////////////
void CropCalc::slideTL(double croptl) {
  croptl = clip(croptl, 0, pseudoSliderMaxTL());
  double oldtl = pseudoSliderValueTL();
  expandLeft(dx*(croptl-oldtl));
  rect.setTop(rect.bottom() - rect.width()/aspect);
  updateAdj();
}
  
void CropCalc::slideTR(double croptr) {
  croptr = clip(croptr, 0, pseudoSliderMaxTR());
  double oldtr = pseudoSliderValueTR();
  expandRight(dx*(croptr-oldtr));
  rect.setTop(rect.bottom() - rect.width()/aspect);
  updateAdj();
}

void CropCalc::slideBL(double cropbl) {
  cropbl = clip(cropbl, 0, pseudoSliderMaxBL());
  double oldbl = pseudoSliderValueBL();
  expandLeft(dx*(cropbl-oldbl));
  rect.setBottom(rect.top() + rect.width()/aspect);
  updateAdj();
}
  
void CropCalc::slideBR(double cropbr) {
  cropbr = clip(cropbr, 0, pseudoSliderMaxBR());
  double oldbr = pseudoSliderValueBR();
  expandRight(dx*(cropbr-oldbr));
  rect.setBottom(rect.top() + rect.width()/aspect);
  updateAdj();
}
  

//////////////////////////////////////////////////////////////////////
void CropCalc::expandLeft(double dx) {
  rect.setLeft(clip(rect.left() - dx, 0, rect.right() - 10));
}

void CropCalc::expandRight(double dx) {
  rect.setRight(clip(rect.right() + dx, rect.left() + 10, osize.width()));
}

void CropCalc::expandTop(double dy) {
  rect.setTop(clip(rect.top() - dy, 0, rect.bottom() - 10));
}

void CropCalc::expandBottom(double dy) {
  rect.setBottom(clip(rect.bottom() + dy, rect.top() + 10, osize.height()));
}
