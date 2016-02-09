// CropCalc.h

#ifndef CROPCALC_H

#define CROPCALC_H

#include "Adjustments.h"
#include <QRectF>
#include <QSize>

class CropCalc {
public:
  enum class Mode { Free, Aspect, Size };
  enum class Orient { Auto, Portrait, Landscape };
public:
  CropCalc();
  void setAll(Adjustments const &adj, QSize osize);
  void setAspect(double aspect, Orient);
  void setFixed(QSize, Orient);
  void moveE(double, Mode);
  void moveN(double, Mode);
  void moveW(double, Mode);
  void moveS(double, Mode);
  void moveNE(double, Mode);
  void moveNW(double, Mode);
  void moveSW(double, Mode);
  void moveSE(double, Mode);
  Adjustments const &adjustments() const;
  QRect cropRect() const;
private:
  Adjustments adj;
  QSize osize;
  QSize fixs;
  QRectF rect;
};

#endif
