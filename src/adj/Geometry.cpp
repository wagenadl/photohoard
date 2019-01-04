// Geometry.cpp

#include "Geometry.h"
#include <math.h>
#include "PDebug.h"
#include "PerspectiveTransform.h"

namespace Geometry {

  PSize croppedSize(PSize osize, Adjustments const &settings) {
    return QSize(osize.width() - settings.cropl - settings.cropr,
                 osize.height() - settings.cropt - settings.cropb);
  }

  PSize scaledCroppedSize(PSize osize, Adjustments const &settings,
                          PSize scaledOSize) {
    return scaledCropRect(osize, settings, scaledOSize).size();
  }

  QRect cropRect(PSize osize, Adjustments const &settings) {
    if (osize.isEmpty())
      return QRect();

    return QRect(QPoint(settings.cropl, settings.cropt),
                 QSize(osize.width()-settings.cropl-settings.cropr,
                       osize.height()-settings.cropt-settings.cropb));
  }

  QRect scaledCropRect(PSize osize, Adjustments const &settings,
                       PSize scaledOSize) {
    if (osize.isEmpty())
      return QRect();

    double xs = scaledOSize.width() / double(osize.width());
    double ys = scaledOSize.height() / double(osize.height());
    QRect orect = QRect(QPoint(settings.cropl, settings.cropt),
                        QSize(osize.width()-settings.cropl-settings.cropr,
                              osize.height()-settings.cropt-settings.cropb));
    return QRect(QPoint(orect.left()*xs+.5, orect.top()*ys+.5),
                 QSize(orect.width()*xs+.5, orect.height()*ys+.5));
  }

  PSize neededScaledOriginalSize(PSize osize, Adjustments const &settings,
                                 PSize desired) { /* static */
    /* We are looking for a size S such that mapCropSize(osize, settings, s)
       fits snugly in DESIRED. Further, S must be a proportional scaling of
       OSIZE.
    */
    PSize fin = croppedSize(osize, settings);
    double fac = fin.scaleFactorToSnuglyFitIn(desired);
    if (fac>1)
      fac = 1; // we won't scale up

    return osize*fac;
    /* I may be stupid, but I cannot conclusively prove that my rounding is
       correct. Therefore, I wrote a little octave script (studyosize.m) that
       confirms that using round-to-nearest always works. */
  }

  QPointF mapToAdjusted(QPointF p, QSize osize, Adjustments const &adj) {
    double w = osize.width();
    double h = osize.height();

    if (adj.rotate) {
      double phi = M_PI*adj.rotate/180;
      QPointF pc = p - QPointF(w/2, h/2);
      pc = QPointF(pc.x()*cos(phi) - pc.y()*sin(phi),
                   pc.x()*sin(phi) + pc.y()*cos(phi));
      p = pc + QPointF(w/2, h/2);
    }

    if (adj.perspv || adj.persph || adj.shearv || adj.shearh) {
      PerspectiveTransform pt(perspectiveTransform(osize, adj));
      p = pt.apply(p);
    }

    if (adj.cropl || adj.cropt) 
      p -= QPointF(adj.cropl, adj.cropt);

    return p;
  }

  QPolygonF mapToAdjusted(QPolygonF pp, QSize osize, Adjustments const &adj) {
    double w = osize.width();
    double h = osize.height();

    if (adj.rotate) {
      double phi = M_PI*adj.rotate/180;
      double cp = cos(phi);
      double sp = sin(phi);
      for (QPointF &p: pp) {
        QPointF pc = p - QPointF(w/2, h/2);
        pc = QPointF(pc.x()*cp - pc.y()*sp,
                     pc.x()*sp + pc.y()*cp);
        p = pc + QPointF(w/2, h/2);
      }
    }

    if (adj.perspv || adj.persph || adj.shearv || adj.shearh) {
      PerspectiveTransform pt(perspectiveTransform(osize, adj));
      for (QPointF &p: pp) 
        p = pt.apply(p);
    }

    if (adj.cropl || adj.cropt) {
      for (QPointF &p: pp)
        p -= QPointF(adj.cropl, adj.cropt);
    }

    return pp;
  }
  
  QPointF mapToScaledAdjusted(QPointF p, QSize osize, Adjustments const &adj,
                              QSize scaledosize) {
    double xs = scaledosize.width() / double(osize.width());
    double ys = scaledosize.height() / double(osize.height());
    p = mapToAdjusted(p, osize, adj);
    return QPointF(xs*p.x(), ys*p.y());
  }

  QPolygonF mapToScaledAdjusted(QPolygonF pp, QSize osize,
                                Adjustments const &adj,
                                QSize scaledosize) {
    double xs = scaledosize.width() / double(osize.width());
    double ys = scaledosize.height() / double(osize.height());
    pp = mapToAdjusted(pp, osize, adj);
    for (QPointF &p: pp)
      p = QPointF(xs*p.x(), ys*p.y());
    return pp;
  }

  bool hasPerspectiveTransform(Adjustments const &adj) {
    return adj.perspv || adj.persph || adj.shearv || adj.shearh;
  }
    
  PerspectiveTransform perspectiveTransform(QSize osize,
                                            Adjustments const &adj) {
    QPolygonF pp;
    double w = osize.width();
    double h = osize.height();
    // top left
    pp << QPointF(w*(-adj.perspv-adj.shearh)/100.0,
                  h*(+adj.persph-adj.shearv)/100.0);
    // top right
    pp << QPointF(w*(100+adj.perspv-adj.shearh)/100.0,
                  h*(-adj.persph+adj.shearv)/100.0);
    // bottom left
    pp << QPointF(w*(adj.perspv+adj.shearh)/100.0,
                  h*(100-adj.persph-adj.shearv)/100.0);
    // bottom right
    pp << QPointF(w*(100-adj.perspv+adj.shearh)/100.0,
                  h*(100+adj.persph+adj.shearv)/100.0);
    
    return PerspectiveTransform(pp, osize);
  }
};
