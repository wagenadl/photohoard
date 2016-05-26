// SlideView.cpp

#include "SlideView.h"
#include <math.h>
#include <QKeyEvent>
#include <QPainter>
#include <QWheelEvent>
#include "PSize.h"
#include "PDebug.h"
#include "CMS.h"
#include "Exif.h"
#include "SlideOverlay.h"
#include "SO_Grid.h"
#include "SO_Layer.h"
#include "ThreadedTransform.h"
#include "PhotoDB.h"

SlideView::SlideView(PhotoDB *db, QWidget *parent): QFrame(parent), db(db) {
  setObjectName("SlideView");
  threadedTransform = new ThreadedTransform(this);
  connect(threadedTransform, SIGNAL(available(quint64, Image16)),
          SLOT(setCMSImage(quint64, Image16)), Qt::QueuedConnection);
  rqid = 0;
  fit = true;
  zoom = 1;
  vsnid = 0;
  makeActions();
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

void SlideView::newImage(quint64 vsn, QSize nat) {
  pDebug() << "SlideView::newImage" << vsn << nat;
  rqid = 0;
  vsnid = vsn;
  naturalSize = nat;
  lastSize = PSize();
  img = Image16(); // might invalidate more gently
  fit = true;
  zoom = 1;
  needLargerImage();
  // we _could_ update() now, which would cause a gray flash
  visualizeLayer(vsn, 0);
}  

void SlideView::updateImage(quint64 vsn, Image16 const &img1, bool force) {
  if (vsn!=vsnid) 
    return;
  pDebug() << "updateImage" << vsn << img1.size() << force;

  if (force || img1.size().exceeds(rqid ? rqsize : img.size())) {
    if (CMS::monitorTransform.isValid()) {
      img = Image16();
      pDebug() << "SV::uI: requesting";
      rqsize = img1.size();
      rqid = threadedTransform->request(img1);
      pDebug() << "SV::uI: requested" << rqid;
    } else {
      rqid = 0;
      img = img1;
      update();
    }
  }
}

void SlideView::setCMSImage(quint64 id, Image16 img1) {
  pDebug() << "SV::setCMSImage" << id << rqid << img1.size();
  if (id==rqid) {
    img = img1;
    rqid = 0;
    update();
  }
}

void SlideView::changeZoomLevel(QPoint, double delta, bool round) {
  double zm = currentZoom() * pow(2.0, delta);
  if (round) 
    zm = pow(2.0, ::round(log(zm)/log(2) / delta) * delta);
  setZoom(zm);
}

void SlideView::setZoom(double z) {
  double z0 = fittingZoom();
  if (z0<1) {
    // fitting zoom is a reduction, we will zoom-to-fit if trying to
    // zoom out farther than that
    if (z<z0) {
      if (!fit)
        scaleToFit();
      return;
    } // otherwise, zoom as requested
  } else {
    // fitting zoom is not a reduction, we will zoom 1:1 if trying to
    // zoom out farther than that
    if (z<1)
      z = 1;
    // otherwise, zoom as requested
  }

  if (fit) {
    fit = false;
    relx = rely = 0.5;
  }
  zoom = z;
  emit newSize(naturalSize.isEmpty() ? size() : naturalSize*zoom);
  update();
  emit newZoom(zoom);
}
  
void SlideView::scaleToFit() {
  fit = true;
  relx = rely = 0.5;
  emit newSize(size());
  update();
  emit newZoom(currentZoom());
}

void SlideView::resizeEvent(QResizeEvent *) {
  if (fit) {
    emit newSize(fit ? size() : naturalSize.isEmpty() ? size()
                 : naturalSize*zoom);
    update();
    emit newZoom(currentZoom());
  }
  for (auto obj: overlays()) {
    SlideOverlay *so = dynamic_cast<SlideOverlay *>(obj);
    ASSERT(so);
    so->resize(size());
  }
  
}

void SlideView::wheelEvent(QWheelEvent *e) {
  changeZoomLevel(e->pos(), e->delta()/1000.0);
}

template<class X> X *findOverlay(QList<SlideOverlay *> const &overs) {
  for (auto ptr: overs) {
    X *so = dynamic_cast<X *>(ptr);
    if (so)
      return so;
  }
  return 0;
}  

void SlideView::makeActions() {
  acts
    << Action { {Qt::SHIFT + int('#'), Qt::SHIFT + Qt::Key_3},
      "Display grid of thirds",
	[this]() {
	SO_Grid *so = findOverlay<SO_Grid>(overlays());
        qDebug() << "SV:so=" << so;
	if (so) 
	  removeOverlay(so);
	else
	  addOverlay(new SO_Grid(this));
	update();
      }};
}

void SlideView::keyPressEvent(QKeyEvent *e) {
  if (acts.activateIf(e)) {
    e->accept();
  } else {
    e->ignore();
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

void SlideView::mouseReleaseEvent(QMouseEvent *e) {
}

void SlideView::mouseMoveEvent(QMouseEvent *e) {
  if (dragging) { 
    updateRelXY(e->pos());
  }
}

void SlideView::mouseDoubleClickEvent(QMouseEvent *) {
  emit doubleClicked();
}

void SlideView::updateRelXY(QPoint p) {
  QPoint dp = p - presspoint;
  QRect r = contentsRect();
  PSize availSize = r.size();
  PSize renderSize = currentImageSize() * zoom;
  PSize targetSize = renderSize - availSize;
  double hscale1 = 1./targetSize.width();
  if (hscale1<1e-4)
    hscale1 = 1e-4;
  double hscale2 = 1./availSize.width();
  double vscale1 = 1./targetSize.height();
  if (vscale1<1e-4)
    vscale1 = 1e-4;
  double vscale2 = 1./availSize.height();
    
  relx = pressrelx - dp.x()*(hscale1 + hscale2);
  rely = pressrely - dp.y()*(vscale1 + vscale2);
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

void SlideView::clear() {
  img = Image16();
  update();
}

void SlideView::paintEvent(QPaintEvent *) {
  if (vsnid==0)
    return;

  if (img.isNull()) {
    needLargerImage();
    return;
  }
  
  QPainter p(this);
  QRect r = contentsRect();
  
  if (fit) {
    bool scaleup = img.size().scaleFactorToSnuglyFitIn(r.size())>1;
    Image16 i1 = img.scaledToFitSnuglyIn(r.size(),
                                   scaleup
                                   ? Image16::Interpolation::NearestNeighbor
                                   : Image16::Interpolation::Linear);
    if (img.width()<naturalSize.width()
        && i1.width()>img.width()
        && img.height()<naturalSize.height()
        && i1.height()>img.height()) {
      // I should only request it if I haven't already
      if (img.size()!=lastSize)
        needLargerImage();
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
    double effZoom = img.size().scaleFactorToSnuglyFitIn(showSize);
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
      sourceRect.setLeft(relx
			 * (showSize.width() - availSize.width()) / effZoom);
      sourceRect.setWidth(availSize.width() / effZoom);
    }
    if (showSize.height()<=availSize.height()) {
      sourceRect.setTop(0);
      sourceRect.setHeight(img.height());
      destRect.setTop((r.top()+r.bottom())/2 - showSize.height()/2);
      destRect.setHeight(img.height() * effZoom);
    } else {
      destRect.setTop(r.top());
      destRect.setHeight(availSize.height());
      sourceRect.setTop(rely
			* (showSize.height() - availSize.height()) / effZoom);
      sourceRect.setHeight(availSize.height() / effZoom);
    }
    Image16 im1 = img.cropped(sourceRect.toRect());
    im1 = im1.scaledToFitSnuglyIn(destRect.size().toSize(), effZoom>1
                     ? Image16::Interpolation::NearestNeighbor
                     : Image16::Interpolation::Linear);
    p.drawImage(destRect.topLeft(), im1.toQImage());
  }
}
    
void SlideView::enterEvent(QEvent *) {
  setFocus();
}

Actions const &SlideView::actions() const {
  return acts;
}

PSize SlideView::currentImageSize() const {
  return naturalSize;
}

quint64 SlideView::currentVersion() const {
  return vsnid;
}

QTransform SlideView::transformationToImage() const {
  return transformationFromImage().inverted();
}

QTransform SlideView::transformationFromImage() const {
  QTransform xf;
  QRect av_r = contentsRect();
  PSize nat_s = currentImageSize();
  PSize av_s = av_r.size();
  if (fit) {
    double scl = nat_s.scaleFactorToSnuglyFitIn(av_s);
    PSize render_s = nat_s.scaledToFitSnuglyIn(av_s);
    xf.translate((av_r.left() + av_r.right())/2 - render_s.width()/2,
		 (av_r.top() + av_r.bottom())/2 - render_s.height()/2);
    xf.scale(scl, scl);
  } else {
    PSize render_s = nat_s * zoom;
    if (render_s.width() <= av_s.width())
      xf.translate((av_r.left() + av_r.right())/2 - render_s.width()/2, 0);
    else
      xf.translate(av_r.left() - relx * (render_s.width() - av_r.width()), 0);
    if (render_s.height() <= av_s.height())
      xf.translate(0, (av_r.top() + av_r.bottom())/2 - render_s.height()/2);
    else
      xf.translate(0, av_r.top() - rely * (render_s.height() - av_r.height()));
    xf.scale(zoom, zoom);
  }
  return xf;
}

void SlideView::addOverlay(SlideOverlay *over) {
  for (auto ptr: overlays())
    if (ptr==over)
      return; // don't add twice
  overlays_ << over;
}

void SlideView::removeOverlay(SlideOverlay *over) {
  auto i = overlays_.begin();
  while (i!=overlays_.end()) {
    auto j = i;
    ++i;
    if (*j == over)
      overlays_.erase(j);
  }
}

QList<SlideOverlay *> SlideView::overlays() const {
  QList<SlideOverlay *> out;
  auto i = overlays_.begin();
  while (i!=overlays_.end()) {
    auto j = i;
    ++i;
    QObject *obj = *j;
    if (obj == 0)
      overlays_.erase(j);
    else
      out << dynamic_cast<SlideOverlay *>(obj);
  }
  return out;
}

void SlideView::needLargerImage() {
  emit needImage(vsnid, desiredSize());
}

void SlideView::visualizeLayer(quint64 vsn, int lay) {
  if (vsn!=vsnid) {
    COMPLAIN("SlideView::visualizeLayer: vsn mismatch");
    return;
  }
  
  qDebug() << "Visualize layer" << vsn << lay;

  SO_Layer *so = findOverlay<SO_Layer>(overlays());
  if (so) 
    removeOverlay(so);

  if (lay>0) {
    so = new SO_Layer(db, this);
    addOverlay(so);
    so->setLayer(vsn, lay);
  }
  update();
}
