// CropCalc.h

#ifndef CROPCALC_H

#define CROPCALC_H

#include "Adjustments.h"
#include <QRectF>
#include <QSize>

class CropCalc {
public:
  enum class Mode { Free, Aspect };//, Size };
  enum class Orient { Auto, Portrait, Landscape };
public:
  CropCalc();
  void reset(Adjustments const &adj, QSize osize);
  void setFree();
  void setAspect(double aspect, Orient o);
  /* SETASPECT - Set fixed aspect ratio for crop rectangle.
     SETASPECT(a, o) sets the fixed aspect ratio of the crop rectangle
     to A, with orientation specified by O. The orientation of A is
     completely ignored, even when O is AUTO. Instead, the orientation
     of the preexisting crop rectangle or of the image is used. (As
     specified through SETALL.)
   */
  //  void setSize(QSize s, Orient o);
  /* SETSIZE - Set fixed size for crop rectangle
     SETSIZE(s, o) sets the fixed size of the crop rectangle to S, with
     orientation specified by O. The orientation of S is completely ignored,
     even when O is AUTO. Instead, the orientation of the preexisting crop
     rectangle or of the image is used. (As specified through SETALL.)
  */
  void slideRight(double);
  void slideTop(double);
  void slideLeft(double);
  void slideBottom(double);
  void slideTL(double);
  void slideTR(double);
  void slideBL(double);
  void slideBR(double);
  Adjustments const &adjustments() const;
  QRect cropRect() const;
  double pseudoSliderValueTL() const;
  double pseudoSliderValueTR() const;
  double pseudoSliderValueBL() const;
  double pseudoSliderValueBR() const;
  double pseudoSliderMaxTL() const;
  double pseudoSliderMaxTR() const;
  double pseudoSliderMaxBL() const;
  double pseudoSliderMaxBR() const;
  double pseudoSliderMaxLeft() const;
  double pseudoSliderMaxRight() const;
  double pseudoSliderMaxTop() const;
  double pseudoSliderMaxBottom() const;
private:
  void updateAdj();
  void setCenterAndSize(QPointF c, QSizeF s);
  double flipIfNeeded(double) const;
  /* FLIPIFNEEDED - Correct aspect ratio
     FLIPIFNEEDED(a), where A is an aspect ratio, returns either A or 1/A:
     If the current crop rectangle has width>height, the number returned by
     FLIPIFNEEDED will be >=1. In the converse situation, it will be <=1.
   */
private:
  void expandTop(double);
  void expandBottom(double); 
  void expandLeft(double);
  void expandRight(double);
private:
  Adjustments adj;
  QSize osize;
  Mode mode;
  double aspect; // w:h
  QRectF rect;
};

#endif
