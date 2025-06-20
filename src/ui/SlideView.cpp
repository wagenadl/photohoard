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
#include <unistd.h>

SlideView::SlideView(PhotoDB *db, QWidget *parent): QFrame(parent), db(db) {
  setObjectName("SlideView");
  threadedTransform = new ThreadedTransform(this);
  connect(threadedTransform, SIGNAL(available(quint64, Image16)),
          SLOT(setCMSImage(quint64, Image16)), Qt::QueuedConnection);
  rqid = 0;
  fit = true;
  zoom = 1;
  showlayers = true;
  suplayers = false;
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
  if (db->acceptReject(vsn)==PhotoDB::AcceptReject::NewImport)
    db->setAcceptReject(vsn, PhotoDB::AcceptReject::Undecided);
  rqid = 0;
  vsnid = vsn;
  naturalSize = nat;
  lastSize = PSize();
  img = Image16(); // might invalidate more gently
  fit = true;
  zoom = 1;
  // pDebug() << "SV::nI: needLargerImage";

  visualizeLayer(vsn, vsn==futvsn ? futlay : 0); // this calls update
  // pDebug() << "SV::nI: done";
}


void SlideView::updateImage(quint64 vsn, Image16 const &img1, bool force,
			    QSize fs) {
  if (vsn!=vsnid) 
    return;
  pDebug() << "SV::updateImage" << vsn << img1.size() << force << fs << rqid << rqsize << img.size();
  if (!fs.isEmpty())
    naturalSize = fs;

  if (force || img1.size().exceeds(rqid ? rqsize : img.size())) {
    if (CMS::monitorTransform.isValid()) {
      // img = Image16(); // this caused massive recursive updates
      pDebug() << "SV::uI: requesting";
      rqsize = img1.size();
      rqid = threadedTransform->request(img1);
      pDebug() << "SV::uI: requested" << rqid;
    } else {
      pDebug() << "SV::uI: will update";
      rqid = 0;
      img = img1;
      update();
    }
  } else if (!fs.isEmpty()) {
    pDebug() << "SV::uI: will update (2)";
    update();
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

QPointF SlideView::relativeImagePoint(QPointF p) const {
  QRect r = contentsRect();
  if (fit) {
    double scale = naturalSize.scaleFactorToSnuglyFitIn(r.size());
    QPointF pcenter((r.right() + r.left())/2, (r.top() + r.bottom())/2);
    return QPointF(0.5 + (p.x()-pcenter.x())/scale / naturalSize.width(),
                   0.5 + (p.y()-pcenter.y())/scale / naturalSize.height());
  } else {
    PSize showSize = naturalSize*zoom;
    PSize availSize = r.size();
    QRectF sourceRect;
    QRectF destRect;
    if (showSize.width()<=availSize.width()) {
      sourceRect.setLeft(0);
      sourceRect.setWidth(naturalSize.width());
      destRect.setLeft((r.left()+r.right())/2 - showSize.width()/2);
      destRect.setWidth(showSize.width());
    } else {
      destRect.setLeft(r.left());
      destRect.setWidth(availSize.width());
      sourceRect.setLeft(relx
			 * (showSize.width() - availSize.width()) / zoom);
      sourceRect.setWidth(availSize.width() / zoom);
    }
    if (showSize.height()<=availSize.height()) {
      sourceRect.setTop(0);
      sourceRect.setHeight(naturalSize.height());
      destRect.setTop((r.top()+r.bottom())/2 - showSize.height()/2);
      destRect.setHeight(showSize.height());
    } else {
      destRect.setTop(r.top());
      destRect.setHeight(availSize.height());
      sourceRect.setTop(rely
			* (showSize.height() - availSize.height()) / zoom);
      sourceRect.setHeight(availSize.height() / zoom);
    }
    double reldestx = (p.x() - destRect.left()) / destRect.width();
    double reldesty = (p.y() - destRect.top()) / destRect.height();
    double srcx = sourceRect.left() + reldestx * sourceRect.width();
    double srcy = sourceRect.top() + reldesty * sourceRect.height();
    return QPointF(srcx/naturalSize.width(),
                   srcy/naturalSize.height());
  }
}
                   
void SlideView::changeZoomLevel(QPointF p, double delta, bool round) {
  QPointF relp0 = relativeImagePoint(p);
  double zm = currentZoom() * pow(2.0, delta);
  if (round) 
    zm = pow(2.0, ::round(log(zm)/log(2) / delta) * delta);
  setZoom(zm);
  if (!fit) {
    // find appropriate relx/rely - there should be no need to do this
    // iteratively, but I cannot right now figure out the closed-form
    // formula
    double scl = .5;
    relx = rely = .5;
    while (scl>.0001) {
      QPointF relp = relativeImagePoint(p);
      if (relp.x() < relp0.x())
        relx += scl;
      else if (relp.x() > relp0.x())
        relx -= scl;
      if (relp.y() < relp0.y())
        rely += scl;
      else if (relp.y() > relp0.y())
        rely -= scl;
      scl /= 2;
      if (relx<0)
        relx = 0;
      else if (relx>1)
        relx = 1;
      if (rely<0)
        rely = 0;
      else if (rely>1)
        rely = 1;
    }
    update();
  }
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
  changeZoomLevel(e->position(), e->angleDelta().y()/1000.0);
}

template<class X> X *findOverlay(SlideView *sv) {
  for (auto ptr: sv->children()) {
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
	SO_Grid *so = findOverlay<SO_Grid>(this);
        //qDebug() << "SV:so=" << so;
	if (so) 
	  delete so;
	else
	  new SO_Grid(this);
	update();
      }};
  acts
    << Action { {Qt::SHIFT + int('@'), Qt::SHIFT + Qt::Key_2},
      "Show/hide layer outlines",
      [this]() {
        qDebug() << "showhide";
        showlayers = !showlayers;
        visualizeLayer(futvsn, futlay);
      }};
}

void SlideView::showHideLayers() {
  showlayers = !showlayers;
  visualizeLayer(futvsn, futlay);
}

void SlideView::suppressLayerOverlay(bool hide) {
  if (suplayers != hide) {
    suplayers = hide;
    visualizeLayer(futvsn, futlay);
  }
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

void SlideView::mouseReleaseEvent(QMouseEvent *) {
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
    pDebug() << "SV:paintEvent" << img.isNull();
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
      needLargerImage();
    }
    p.drawImage(QPoint((r.left() + r.right())/2 - i1.width()/2,
                       (r.top() + r.bottom())/2 - i1.height()/2),
		i1.toQImage());
  } else {
    PSize showSize = naturalSize*zoom;
    PSize availSize = r.size();
    double effZoom = img.size().scaleFactorToSnuglyFitIn(showSize);
    // pDebug() << "slideview paint" << showSize << availSize << img.size() << effZoom << effZoom*img.size() << naturalSize;
    QRectF sourceRect;
    QRectF destRect;
    if (!img.size().isLargeEnoughFor(showSize)
	&& naturalSize.exceeds(img.size())) {
      emit needLargerImage();
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
    // pDebug() << "SlideView" << scl << render_s;
    xf.translate((av_r.left() + av_r.right())/2 - render_s.width()/2,
		 (av_r.top() + av_r.bottom())/2 - render_s.height()/2);
    xf.scale(scl, scl);
  } else {
    PSize render_s = nat_s * zoom;
    // pDebug() << "SlideView" << zoom << render_s;
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


QList<SlideOverlay *> SlideView::overlays() const {
  QList<SlideOverlay *> out;
  for (auto *c: children()) {
    SlideOverlay *so = dynamic_cast<SlideOverlay *>(c);
    if (so)
      out << so;
  }
  return out;
}

void SlideView::needLargerImage() {
  QSize ds = desiredSize();
  pDebug() << "needLargerImage" << vsnid << ds;
  if (ds != lastSize) {
    pDebug() << "  requesting larger image";
    emit needImage(vsnid, desiredSize());
    lastSize = ds;
  } else {
    pDebug() << "  not repeating previous request";
  }
}

void SlideView::visualizeLayer(quint64 vsn, int lay) {
  bool replace = futvsn != vsn || futlay != lay;
  futvsn = vsn;
  futlay = lay;

  if (vsn!=vsnid) {
    pDebug() << "SlideView::visualizeLayer: vsn mismatch" << vsn
    	     << "exp" << vsnid;
    return;
  }

  SO_Layer *so = findOverlay<SO_Layer>(this);
  //  qDebug() << "old layer" << so;
  if (so && replace) {
    delete so;
    so = 0;
  }
  
  if (lay>0 && showlayers && !suplayers) {
    if (so) {
      so->show();
    } else {
      so = new SO_Layer(db, this);
      connect(so, &SO_Layer::layerMaskChanged,
              this, &SlideView::layerMaskChanged);
      so->setLayer(vsn, lay);
    }
  } else {
    if (so)
      so->hide();
  }
  update();
}
