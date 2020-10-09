// PerspectiveTransform.cpp

#include "PerspectiveTransform.h"
#include <opencv2/imgproc/imgproc.hpp>
#include "PDebug.h"

class PT_Data: public QSharedData {
public:
  cv::Mat m;
};

PerspectiveTransform::PerspectiveTransform(): d(new PT_Data) {
  d->m = cv::Mat::eye(3,3, CV_32FC1);
}

PerspectiveTransform::PerspectiveTransform(QPolygonF poly, QSize osize):
  d(new PT_Data) {
  if (poly.size()!=4) {
    COMPLAIN("PerspectiveTransform: need 4-gon");
    return;
  }
  cv::Point2f src[4];
  cv::Point2f dst[4];
  for (int k=0; k<4; k++)
    dst[k] = cv::Point2f(poly[k].x(), poly[k].y());
  src[0] = cv::Point2f(0, 0);
  src[1] = cv::Point2f(osize.width(), 0);
  src[2] = cv::Point2f(0, osize.height());
  src[3] = cv::Point2f(osize.width(), osize.height());
 
  d->m = cv::getPerspectiveTransform(dst, src);
}

PerspectiveTransform::~PerspectiveTransform() {
}

PerspectiveTransform PerspectiveTransform::inverse() const {
  PerspectiveTransform inv;
  inv.d->m = d->m.inv();
  return inv;
}

PerspectiveTransform::PerspectiveTransform(PerspectiveTransform const &o) {
  d = o.d;
}

PerspectiveTransform &PerspectiveTransform::operator=(PerspectiveTransform const &o) {
  if (this == &o)
    return *this;
  d = o.d;
  return *this;
}


Image16 PerspectiveTransform::warp(Image16 const &img,
                                   Image16::Interpolation i) const {
  if (i!=Image16::Interpolation::NearestNeighbor) {
    if (i!=Image16::Interpolation::Linear)
      COMPLAIN("Note: PerspectiveTransform::warp only supports" 
               " up to linear interpolation");
    i = Image16::Interpolation::Linear;
  }
  int cvfmt = Image16::cvFormat(img.format());
  cv::Mat const in(img.height(), img.width(), cvfmt,
                   (void*)img.bytes(), img.bytesPerLine());
  Image16 res(img.size(), img.format());
  cv::Mat out(img.height(), img.width(), cvfmt, res.bytes(), res.bytesPerLine());
  cv::warpPerspective(in, out, d->m, out.size(),
                      Image16::cvInterpolation(i),
                      cv::BORDER_CONSTANT, cv::Scalar());
  // I should specify the value of that scalar.
  return res;
}

QPointF PerspectiveTransform::apply(QPointF const &p) const {
  cv::Mat pm = cv::Mat_<double>(3,1);
  pm.at<double>(0,0) = p.x();
  pm.at<double>(1,0) = p.y();
  pm.at<double>(2,0) = 1;
  pm = d->m * pm;
  return QPointF(pm.at<double>(0,0)/pm.at<double>(0,2),
                 pm.at<double>(1,0)/pm.at<double>(0,2));
}
