// PSize.h

#ifndef PSIZE_H

#define PSIZE_H

#include <QSize>

class PSize: public QSize {
  /* PSIZE - Enhancements of QSize
     PSIZE adds many useful functions to QSize.
   */
public:
  PSize(): QSize(0, 0) { }
  PSize(QSize const &s): QSize(s) { }
  PSize(int w, int h): QSize(w, h) { }
  bool contains(QSize const &b) const {
  // CONTAINS - a.CONTAINS(b) is true if both dimensions of A are at least as large as the corresponding dimension of B.
    return width()>=b.width() && height()>=b.height(); }
  bool isContainedIn(QSize const &b) const {
  // ISCONTAINEDIN - a.ISCONTAINEDIN(b) is true if b.CONTAINS(a) is true.
    return width()<=b.width() && height()<=b.height(); }
  bool exceeds(QSize const &b) const {
  // EXCEEDS - a.EXCEEDS(b) is true if b.CONTAINS(a) is not true.
    return width()>b.width() || height()>b.height(); }
  bool isExceededBy(QSize const &b) const {
  // ISEXCEEDEDBY - a.ISEXCEEDEDBY(b) is true if b.ISEXCEEDEDBY(a) is true.
    return b.width()>width() || b.height()>height(); }
  bool isLargeEnoughFor(QSize const &b) const {
  /* ISLARGEENOUGHFOR - Test if a size is large enough to fill a space.
     a.ISLARGEENOUGHFOR(b) is true if an image of size A would not
     have to be scaled up to fit in B. In other words, if at least one
     dimension of A is no smaller than the corresponding dimension of
     B.
  */
    return width()>=b.width() || height()>=b.height();  }
  bool snuglyFitsIn(QSize const &b) const {
  /* SNUGLYFITSIN - a.SNUGLYFITSIN(b) is true if an image of size B is the perfect size to fit in B.
     In other words, at least one dimension of A is identical to the
     corresponding dimension of B and the other dimension of A is not greater
     than the corresponding dimension of B.
    */
    return isContainedIn(b) && isLargeEnoughFor(b); }
  bool operator<(QSize const &b) const {
  // OPERATOR< - a<b if the area of A is less than the area of B
    return width()*height() < b.width()*b.height(); }
  bool operator>(QSize const &b) const {
  // OPERATOR> - a>b if the area of A is greater than the area of B
    return width()*height() > b.width()*b.height(); }
  PSize &operator|=(QSize const &s);
  // OPERATOR|= - a |= b expands the area of A to contain B.
  PSize operator*(double s) const { return PSize(width()*s+.5, height()*s+.5); }
  PSize scaledToFitSnuglyIn(QSize const &b) const;
  // SCALEDTOFITSNUGLYIN - a.SCALEDTOFITSNUGLYIN(b) scales A up or down such that the result SNUGLYFITSIN(b).
  PSize scaledDownToFitIn(QSize const &b) const;
  /* SCALEDDOWNTOFITIN - Scale size down to fit in a space.
     If A already fits in B, a.SCALEDDOWNTOFITIN(b) simply returns B. Otherwise, it returns a scaled down version that does fit snugly.
   */
  PSize scaledToSnuglyContain(QSize const &b) const;
  PSize scaledUpToContain(QSize const &b) const;
  double scaleFactorToSnuglyFitIn(QSize const &b) const;
  double scaleDownFactorToFitIn(QSize const &b) const;
  double scaleFactorToSnuglyContain(QSize const &b) const;
  double scaleUpFactorToContain(QSize const &b) const;
  PSize rotated90() const;
  void rotate90();
  int maxDim() const { return width()>height() ? width() : height(); }
  static PSize square(int w) { return PSize(w, w); }
};

Q_DECLARE_TYPEINFO(PSize, Q_MOVABLE_TYPE);

#endif
