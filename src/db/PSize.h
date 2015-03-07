// PSize.h

#ifndef PSIZE_H

#define PSIZE_H

#include <QSize>

class PSize: public QSize {
public:
  PSize(): QSize(0, 0) { }
  PSize(QSize const &s): QSize(s) { }
  PSize(int w, int h): QSize(w, h) { }
  bool contains(QSize const &s) const {
    return width()>=s.width() && height()>=s.height(); }
  bool containedIn(QSize const &s) const {
    return width()<=s.width() && height()<=s.height(); }
  bool exceeds(QSize const &s) const {
    return width()>s.width() || height()>s.height(); }
  bool exceededBy(QSize const &s) const {
    return s.width()>width() || s.height()>height(); }
  bool isLargeEnoughFor(QSize const &s) const {
    /* Returns true if scaling us to fit in s would not scale us up. */
    return width()>=s.width() || height()>=s.height();
  }
  bool operator<(QSize const &s) const {
    // Smaller in area
    return width()*height() < s.width()*s.height();
  }
  PSize &operator|=(QSize const &s);
  PSize operator*(double s) const { return PSize(width()*s, height()*s); }
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
  int maxDim() const { return width()>height() ? width() : height(); }
  static PSize square(int w) { return PSize(w, w); }
};

Q_DECLARE_TYPEINFO(PSize, Q_MOVABLE_TYPE);

#endif
