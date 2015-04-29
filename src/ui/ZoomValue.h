// ZoomValue.h

#ifndef ZOOMVALUE_H

#define ZOOMVALUE_H

#include <math.h>

class ZoomValue {
public:
  ZoomValue(): z(1) { }
  ZoomValue(double z): z(z) { }
  operator double() const { return z; }
  ZoomValue &operator=(double z1) { z=z1; return *this; }
  void halfStepUp() { setToNearestHalfStep(z*1.414); }
  void halfStepDown() { setToNearestHalfStep(z/1.414); }
  void wholeStepUp() { setToNearestStep(z*2); }
  void wholeStepDown() { setToNearestStep(z/2); }
  void setToNearestHalfStep(double z1) {
    double lz = log2(z1);
    double lz0 = round(lz);
    if (lz>lz0+.25)
      z = halfStepAbove(lz0);
    else if (lz<lz0-.25)
      z = halfStepBelow(lz0);
    else
      z = pow(2, lz0);
    enforceLimits();
  }
  void setToNearestStep(double z1) {
    z = pow(2, round(log2(z1)));
    enforceLimits();
  }
  void enforceLimits() {
    if (z>=lowerLimit() && z<=upperLimit())
      return;
    if (z<lowerLimit())
      z = lowerLimit();
    else if (z>upperLimit())
      z = upperLimit();
    else
      z = 1;
  }
  static double lowerLimit() { return 0.05; }
  static double upperLimit() { return 10; }
  static double log2(double z) { return log(z)/log(2); }
  static double halfStepAbove(double logz) {
    if (logz>=0)
      return pow(2, logz) * 3./2;
    else
      return pow(2, logz) * 4./3;
  }
  static double halfStepBelow(double logz) {
    if (logz>=1)
      return pow(2, logz) * 3./4;
    else
      return pow(2, logz) * 2./3;
  }
private:
  double z;
};

#endif
