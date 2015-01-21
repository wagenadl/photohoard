// ImageSpaceConverters.cpp

#include "ImageSpaceConverters.h"

#include <QImage>

namespace ImageSpaceConverters {
  constexpr float one = 16384.0f;
  
  void linearToSRGB(short *dst, short const *src, int N) {
    N *= 3;
    while (N--) {
      float y = *src++ / one;
      if (y>.0031308)
        *dst++ = short(one*(1.055*pow(y, 1/2.4) - 0.055));
      else
        *dst++ = short(one*y*12.92);
    }
  }

  void sRGBToLinear(short *dst, short const *src, int N) {
    N *= 3;
    while (N--) {
      float y = *src++ / one;
      if (y>.04045)
        *dst++ = ushort(one*pow((y+0.055)/1.055, 2.4));
      else
        *dst++ = ushort(one*y/12.92);
    }
  }

  inline float labfinv(float x) {
    if (x>6/29.f)
      return powf(x, 3);
    else
      return 3*(6/29.f)*(6/29.f) * (x - 4/29.f);
  }

  void labD50ToXYZ(short *dst, short const *src, int N) {
    constexpr float scl = 100/one;
    while (N--) {
      float L0 = (1/116.f) * src[0]*scl + 16;
      float Y = labfinv(L0);
      float X = 0.9642f * labfinv(L0 + src[1]*scl / 500);
      float Z = 0.8251f * labfinv(L0 + src[2]*scl / 200);
      dst[0] = short(X*one);
      dst[1] = short(Y*one);
      dst[2] = short(Z*one);
      src += 3;
      dst += 3;
    }
  }

  void linearRGBToXYZ(short *dst, short const *src, int N) {
    while (N--) {
      float R = *src++;
      float G = *src++;
      float B = *src++;
      *dst++ = short(.4124f*R + .3576f*G + .1805f*B);
      *dst++ = short(.2126f*R + .7152f*G + .0722f*B);
      *dst++ = short(.0193f*R + .1192f*G + .9505f*B);
    }
  }

  void xyzToLinearRGB(short *dst, short const *src, int N) {
    while (N--) {
      float X = *src++;
      float Y = *src++;
      float Z = *src++;
      *dst++ = short(3.2406*X - 1.5372*Y - 0.4986*Z);
      *dst++ = short(-.9689*X + 1.8758*Y + 0.0415*Z);
      *dst++ = short(0.0557*X - 0.2040*Y + 1.0570*Z);
    }
  }

  inline float labf(float x) {
    if (x>6*6*6/(29*29*29.f))
      return powf(x, 1/3.f);
    else
      return (29*29/(6*6*3.f)) * x + 4/29.f;
  }

  void xyzToLabD50(short *dst, short const *src, int N) {
    constexpr float scl = one/100;
    while (N--) {
      float X = *src++ / one;
      float Y = *src++ / one;
      float Z = *src++ / one;
      //    xyz = [0.9642    1.0000    0.8251];
      float L0 = labf(Y);
      float Lstar = 116*L0 - 16;
      float astar = 500 * (labf(X/0.9642) - L0);
      float bstar = 200 * (L0 - labf(Z/0.8251));
      *dst++ = short(Lstar * scl);
      *dst++ = short(astar * scl);
      *dst++ = short(bstar * scl);
    }
  }
};  
