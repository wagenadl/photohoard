// ImageSpaceConverters.cpp

#include "ImageSpaceConverters.h"

#include <QImage>

namespace ImageSpaceConverters {
void linearToSRGB(uchar *dst, uchar const *src, int N) {
  static uchar lookup[256];
  static bool inited = false;
  if (!inited) {
    for (int x=0; x<256; x++) {
      double y = x/255.0;
      if (y>.0031308)
        lookup[x] = uchar(255*(1.055*pow(y, 1/2.4) - 0.055));
      else
        lookup[x] = uchar(255*y*12.92);
    }
    inited = true;
  }
  while (N--)
    *dst++ = lookup[*src++];
}

void linearToSRGB(ushort *dst, ushort const *src, int N) {
  while (N--) {
    float y = *src++ / 65535.0;
    if (y>.0031308)
      *dst++ = ushort(65535*(1.055*pow(y, 1/2.4) - 0.055));
    else
      *dst++ = ushort(65535*y*12.92);
  }
}

void sRGBToLinear(uchar *dst, uchar const *src, int N) {
  static uchar lookup[256];
  static bool inited = false;
  if (!inited) {
    for (int x=0; x<256; x++) {
      double y = x/255;
      if (y>.04045)
        lookup[x] = uchar(pow((y+0.055)/1.055, 2.4));
      else
        lookup[x] = uchar(255*y/12.92);
    }
    inited = true;
  }
  while (N--)
    *dst++ = lookup[*src++];
}

void sRGBToLinear(ushort *dst, ushort const *src, int N) {
  while (N--) {
    float y = *src++ / 65535.0;
    if (y>.04045)
      *dst++ = ushort(65535*pow((y+0.055)/1.055, 2.4));
    else
      *dst++ = ushort(65535*y/12.92);
  }
}

};  
