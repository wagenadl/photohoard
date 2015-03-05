// PSize.cpp

#include "PSize.h"
#include <limits>

PSize PSize::scaledDownToFitIn(QSize const &s) const {
  if (isEmpty() || s.isEmpty())
    return *this;
  double xf = s.width() / double(width());
  double yf = s.height() / double(height());
  if (xf<yf) {
    if (xf>=1)
      return *this;
    else
      return PSize(s.width(), int(height()*xf + 0.5));
  } else {
    if (yf>=1)
      return *this;
    else
      return PSize(int(width()*yf + 0.5), s.height());
  }  
}

PSize PSize::scaledToFitIn(QSize const &s) const {
  if (isEmpty() || s.isEmpty())
    return *this;
  double xf = s.width() / double(width());
  double yf = s.height() / double(height());
  if (xf<yf) 
    return PSize(s.width(), int(height()*xf + 0.5));
  else 
    return PSize(int(width()*yf + 0.5), s.height());
}
  
PSize PSize::scaledToContain(QSize const &s) const {
  if (isEmpty() || s.isEmpty())
    return *this;
  double xf = s.width() / double(width());
  double yf = s.height() / double(height());
  if (xf>yf) {
    if (xf<=1)
     return *this;
    else
      return PSize(s.width(), int(height()*xf + 0.5));
  } else {
    if (yf<=1)
      return *this;
    else
      return PSize(int(width()*yf + 0.5), s.height());
  }
}

PSize PSize::scaledUpToContain(QSize const &s) const {
  if (isEmpty() || s.isEmpty())
    return *this;
  double xf = s.width() / double(width());
  double yf = s.height() / double(height());
  if (xf>yf) 
    return PSize(s.width(), int(height()*xf + 0.5));
  else 
    return PSize(int(width()*yf + 0.5), s.height());
}

double PSize::scaleFactorToFitIn(QSize const &s) const {
  if (isEmpty())
    return 1;
  else if (s.isEmpty())
    return 0;
  double xf = s.width() / double(width());
  double yf = s.height() / double(height());
  return xf<yf ? xf : yf;
}
  
double PSize::scaleDownFactorToFitIn(QSize const &s) const {
  double f = scaleFactorToFitIn(s);
  return f<1 ? f : 1;
}

double PSize::scaleFactorToContain(QSize const &s) const {
  if (s.isEmpty())
    return isEmpty() ? 1 : 0;
  else if (isEmpty())
    return std::numeric_limits<double>::inifity();
}

double PSize::scaleUpFactorToContain(QSize const &s) const {
  double f = scaleFactorToContain(s);
  return f>1 ? f : 1;
}

PSize PSize::rotated90() const {
  return PSize(height(), width());
}

void PSize::rotate90() {
  *this = PSize(height(), width());
}
