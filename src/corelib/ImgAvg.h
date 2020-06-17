// ImgAvg.h

// debug code to report average pixel of image

#include "Image16.h"

inline QString averagePixel(Image16 const &img1) {
  QImage img = img1.toQImage();
  int r = 0;
  int g = 0;
  int b = 0;
  int a = 0;
  int n = 0;
  int W = img.width();
  for (int y=0; y<img.height(); ++y) {
    uint8_t const *src = img.scanLine(y);
    for (int x=0; x<W; ++x) {
      b += *src++;
      g += *src++;
      r += *src++;
      a += *src++;
      n++;
    }
  }
  if (n>0) {
    b /= n;
    g /= n;
    r /= n;
    a /= n;
  }
  return QString("(%1,%2,%3)").arg(r).arg(g).arg(b);
}

