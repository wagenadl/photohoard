// ImageSpaceConverters.cpp

#include "ImageSpaceConverters.h"

#include <QImage>
#include <QDebug>

namespace ImageSpaceConverters {
  constexpr float one = 16384.0f;
  
  void linearToSRGB(short *dst, short const *src, int X, int Y, int L) {
    int dL = L - 3*X;
    qDebug() << "linearToSRGB" << X << Y << L << dL;
    while (Y--) {
      for (int x=0; x<3*X; x++) {
        float y = *src++ / one;
        if (y>.0031308)
          *dst++ = short(one*(1.055*pow(y, 1/2.4) - 0.055));
        else
          *dst++ = short(one*y*12.92);
      }
      src += dL;
      dst += dL;
    }
  }

  void sRGBToLinear(short *dst, short const *src, int X, int Y, int L) {
    int dL = L - 3*X;
    qDebug() << "SRGBTOLINEAR" << X << Y << L << dL;
    while (Y--) {
      for (int x=0; x<3*X; x++) {
        float y = *src++ / one;
        if (y>.04045)
          *dst++ = ushort(one*pow((y+0.055)/1.055, 2.4));
        else
          *dst++ = ushort(one*y/12.92);
      }
      src += dL;
      dst += dL;
    }
  }

  inline float labfinv(float x) {
    if (x>6/29.f)
      return powf(x, 3);
    else
      return (3*6*6/(29*29.f)) * (x - 4/29.f);
  }

  void labD50ToXYZ(short *dst, short const *src, int X, int Y, int L) {
    constexpr float scl = 100/one;
    int dL = L - 3*X;
    qDebug() << "labd50toxyz" << X << Y << L << dL;
    while (Y--) {
      for (int x=0; x<X; x++) {
        float L0 = (1/116.f) * (*src++ * scl + 16);
        float Y_ = labfinv(L0);
        float X_ = 0.9642f * labfinv(L0 + *src++ * scl / 500);
        float Z_ = 0.8251f * labfinv(L0 - *src++ * scl / 200);
        *dst++ = short(X_*one);
        *dst++ = short(Y_*one);
        *dst++ = short(Z_*one);
      }
      src += dL;
      dst += dL;
    }
  }

  void linearRGBToXYZ(short *dst, short const *src, int X, int Y, int L) {
    int dL = L - 3*X;
    qDebug() << "linearrgbtoxyz" << X << Y << L << dL;
    while (Y--) {
      for (int x=0; x<X; x++) {
        float R = *src++;
        float G = *src++;
        float B = *src++;
        *dst++ = short(.4124f*R + .3576f*G + .1805f*B);
        *dst++ = short(.2126f*R + .7152f*G + .0722f*B);
        *dst++ = short(.0193f*R + .1192f*G + .9505f*B);
      }
      src += dL;
      dst += dL;
    }
  }

  void xyzToLinearRGB(short *dst, short const *src, int X, int Y, int L) {
    int dL = L - 3*X;
    qDebug() << "xyztolinearrgb" << X << Y << L << dL;
    while (Y--) {
      for (int x=0; x<X; x++) {
        float X_ = *src++;
        float Y_ = *src++;
        float Z_ = *src++;
        *dst++ = short(3.2406*X_ - 1.5372*Y_ - 0.4986*Z_);
        *dst++ = short(-.9689*X_ + 1.8758*Y_ + 0.0415*Z_);
        *dst++ = short(0.0557*X_ - 0.2040*Y_ + 1.0570*Z_);
      }
      src += dL;
      dst += dL;
    }
  }

  inline float labf(float x) {
    if (x>6*6*6/(29*29*29.f))
      return powf(x, 1/3.f);
    else
      return (29*29/(6*6*3.f)) * x + 4/29.f;
  }

  void xyzToLabD50(short *dst, short const *src, int X, int Y, int L) {
    constexpr float scl = one/100;
    int dL = L - 3*X;
    qDebug() << "xyztolabd50" << X << Y << L << dL;
    while (Y--) {
      for (int x=0; x<X; x++) {
        float X_ = *src++ / one;
        float Y_ = *src++ / one;
        float Z_ = *src++ / one;
        //    xyz = [0.9642    1.0000    0.8251];
        float L0 = labf(Y_);
        float Lstar = 116*L0 - 16;
        float astar = 500 * (labf(X_/0.9642f) - L0);
        float bstar = 200 * (L0 - labf(Z_/0.8251f));
        if (fabs(astar)>100 || fabs(Lstar)>100 || fabs(bstar)>100)
          qDebug() << X_ << Y_ << Z_ << "->" << L0 
                   << "->" << Lstar << astar << bstar;
        *dst++ = short(Lstar * scl);
        *dst++ = short(astar * scl);
        *dst++ = short(bstar * scl);
      }
      src += dL;
      dst += dL;
    }
  }
}
