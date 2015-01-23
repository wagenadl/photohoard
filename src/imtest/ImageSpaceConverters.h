// ImageSpaceConverters.h

#ifndef IMAGESPACECONVERTERS_H

#define IMAGESPACECONVERTERS_H

namespace ImageSpaceConverters {
  void linearToSRGB(short *, short const *, int X, int Y, int L);
  void sRGBToLinear(short *, short const *, int X, int Y, int L);
  void labD50ToXYZ(short *, short const *, int X, int Y, int L);
  void linearRGBToXYZ(short *, short const *, int X, int Y, int L);
  void xyzToLinearRGB(short *, short const *, int X, int Y, int L);
  void xyzToLabD50(short *, short const *, int X, int Y, int L);
}

#endif
