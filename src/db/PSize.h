// PSize.h

#ifndef PSIZE_H

#define PSIZE_H

#include <QSize>

class PSize: public QSize {
public:
  PSize(QSize const &s): QSize(s) { }
  PSize(int w, int h): QSize(w, h) { }
  bool contains(QSize const &s) const {
    return width()>=s.width() && height()>=s.height(); }
  bool containedIn(QSize const &s) const {
    return width()<=s.width() && height()<=s.height(); }
  bool exceeds(QSize const &s) const {
    return width()>s.width() || height()>=s.height(); }
  bool exceededBy(QSize const &s) const {
    return s.width()>width() || s.height()>height(); }
  PSize scaledDownToFitIn(QSize const &s) const;
  PSize scaledToFitIn(QSize const &s) const;
  PSize scaledToContain(QSize const &s) const;
  PSize scaledUpToContain(QSize const &s) const;
  double scaleFactorToFitIn(QSize const &s) const;
  double scaleDownFactorToFitIn(QSize const &s) const;
  double scaleFactorToContain(QSize const &s) const;
  double scaleUpFactorToContain(QSize const &s) const;
  PSize rotated90() const;
  void rotate90();
};

#endif
