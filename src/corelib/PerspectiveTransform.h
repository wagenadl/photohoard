// PerspectiveTransform.h

#ifndef PERSPECTIVETRANSFORM_H

#define PERSPECTIVETRANSFORM_H

#include <QPolygonF>
#include <QSize>
#include "Image16.h"
#include <QSharedDataPointer>

class PerspectiveTransform {
public:
  PerspectiveTransform(QPolygonF corners, QSize osize);
  PerspectiveTransform();
  ~PerspectiveTransform();
  PerspectiveTransform(PerspectiveTransform const &);
  PerspectiveTransform &operator=(PerspectiveTransform const &);
  QPointF apply(QPointF const &) const;
  Image16 warp(Image16 const &img,
               Image16::Interpolation=Image16::Interpolation::Linear) const;
  PerspectiveTransform inverse() const;
private:
  QSharedDataPointer<class PT_Data> d;
};

#endif
