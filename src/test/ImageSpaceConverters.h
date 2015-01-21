// ImageSpaceConverters.h

#ifndef IMAGESPACECONVERTERS_H

#define IMAGESPACECONVERTERS_H

namespace ImageSpaceConverters {
  void linearToSRGB(uchar *, uchar const *, int N);
  void linearToSRGB(ushort *, ushort const *, int N);
  void sRGBToLinear(uchar *, uchar const *, int N);
  void sRGBToLinear(ushort *, ushort const *, int N);

  inline float labfinv(float x) {
    if (x>6/29.f)
      return powf(x, 3);
    else
      return 3*(6/29.f)*(6/29.f) * (x - 4/29.f);
  }
  
  template <typename T> void labD50ToXYZ(T *dst, T const *src, int N, int dX,
                                         float maxT) {
    while (N--) {
      float L0 = (1/116.f) * (src[0]*(100/maxT)) + 16;
      float Y = labfinv(L0);
      float X = 0.9642f * labfinv(L0 + src[1]*(128/maxT)/500);
      float Z = 0.8251f * labfinv(L0 + src[1]*(128/maxT)/200);
      dst[0] = T(X*maxT);
      dst[1] = T(Y*maxT);
      dst[2] = T(Z*maxT);
      src += dX;
      dst += dX;
    }
  }

  template <typename T> void linearRGBToXYZ(T *dst, T const *src,
                                            int N, int dX) {
    while (N--) {
      T R = src[0];
      T G = src[1];
      T B = src[2];
      dst[0] = T(.4124f*R + .3576f*G + .1805f*B);
      dst[1] = T(.2126f*R + .7152f*G + .0722f*B);
      dst[2] = T(.0193f*R + .1192f*G + .9505f*B);
      // This is no good. RGB=111 goes to XYZ=(.95,1, 1.09).
      // I clearly must use 1.14 bits for this to work nicely.
      src += dX;
      dst += dX;
    }
  }
}

#endif
