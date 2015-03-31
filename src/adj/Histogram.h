// Histogram.h

#ifndef HISTOGRAM_H

#define HISTOGRAM_H

#include "Image16.h"
#include <QVector>

class Histogram {
public:
  Histogram();
  void clear();
  void setImage(Image16 const &);
  void setNumberOfBins(int);
  void setColorSpace(Image16::Format);
  /* setNumberOfBins and setColorSpace may not take effect until the next time
     setImage is called. */
  int numberOfBins() const; // actual number, not future
  Image16::Format colorSpace() const; // actual
  QVector<double> const &channel(int k) const;
  /* Histogram for channel k; numbers are fractions per bin.
     Bins span the entire space, which means that for XYZ and XYZp, they
     go up to 2 rather than 1. */
  /* Current implementation is naive about signed numbers.
     But that will change. */
private:
  void recalculate() const;
  bool isInstantiated() const;
private:
  mutable int nbins;
  int nextbins;
  mutable Image16::Format fmt;
  Image16::Format nextfmt;
  mutable QVector<double> hst[3];
  mutable Image16 image;
};

#endif
