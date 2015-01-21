// ImageSpaceConverters.h

#ifndef IMAGESPACECONVERTERS_H

#define IMAGESPACECONVERTERS_H

namespace ImageSpaceConverters {
  void linearToSRGB(short *, short const *, int N);
  void sRGBToLinear(short *, short const *, int N);
  void labD50ToXYZ(short *, short const *, int N);
  void linearRGBToXYZ(short *, short const *, int N);
  void xyzToLinearRGB(short *, short const *, int N);
  void xyzToLabD50(short *, short const *, int N);
}

#endif
