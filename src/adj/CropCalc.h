// CropCalc.h

#ifndef CROPCALC_H

#define CROPCALC_H

#include "Adjustments.h"
#include <QRectF>
#include <QSize>
#include "CropEnums.h"

class CropCalc {
public:
  CropCalc();
  void reset(Adjustments const &adj, QSize osize);
  void setValue(QString k, double v); // tweak adj.
  void setFree();
  void setFixedAspect();
  void setOrient(Orient o);
  /* SETORIENT - Set orientation
     SETORIENT(o) changes the orientation of the crop rectangle without
     changing the aspect ratio.
     SETORIENT(Orient::Auto) does nothing except setting mode.
   */
  void setAspect(double aspect);
  void setAspect(double aspect, Orient o);
  /* SETASPECT - Set fixed aspect ratio for crop rectangle.
     SETASPECT(a, o) sets the fixed aspect ratio of the crop rectangle
     to A, with orientation specified by O. The orientation of A is
     completely ignored, even when O is AUTO. Instead, the orientation
     of the preexisting crop rectangle or of the image is used. (As
     specified through SETALL.)
   */
  CropMode cropMode() const;
  double aspectRatio() const;
  QSize originalSize() const;
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
  void optimize();
private:
  void updateAdj();
  void setCenterAndSize(QPointF c, QSizeF s);
  double flipIfNeeded(double) const;
  /* FLIPIFNEEDED - Correct aspect ratio
     FLIPIFNEEDED(a), where A is an aspect ratio, returns either A or 1/A:
     If the current crop rectangle has width>height, the number returned by
     FLIPIFNEEDED will be >=1. In the converse situation, it will be <=1.
   */
  void calcDxy();
private:
  void expandTop(double);
  void expandBottom(double); 
  void expandLeft(double);
  void expandRight(double);
private:
  Adjustments adj;
  QSize osize;
  CropMode mode;
  double aspect; // w:h
  double dx, dy;
  QRectF rect;
};

#endif
