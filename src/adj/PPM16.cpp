// PPM16.cpp

#include "PPM16.h"

static void skipWhite(uchar const *&src, int &len) {
  bool incomment = false;
  while (len) {
    if (*src>'#' && !incomment)
      return;
    else if (*src=='#') 
      incomment = true;
   else if (*src==13 || *src==10)
     incomment = false;
    src++;
    len--;
  }
}

static void skipToWhite(uchar const *&src, int &len) {
  while (len && *src>'#') {
    src++;
    len--;
  }
}

static int getNumber(uchar const *src, int len) {
  int res = 0;
  while (len && *src>='0' && *src<=9) {
    res = 10*res;
    res += *src-'0';
    src++;
    len--;
  }
  return res;
}

PPM16::PPM16(QByteArray const &ar) {
  w = h = 0;
  uchar const *src = (uchar const *)ar.data();
  int len = ar.size();
  if (len<3)
    return;
  if (src[0]!='P' || src[1]!='6' || src[2]>'#')
    return;
  src += 3; len -= 3;
  skipWhite(src, len);
  int wid = getNumber(src, len);
  skipToWhite(src, len);
  skipWhite(src, len);
  int hei = getNumber(src, len);
  skipToWhite(src, len);
  skipWhite(src, len);
  int dep = getNumber(src, len);
  skipToWhite(src, len);
  src++;
  len--;
  if (wid<=0 || hei<=0 || dep!=65535 || len<=0)
    return;

  w = wid;
  h = hei;
  dat = QImage(w*3, h, QImage::Format_RGB16);
  uchar *dst = dat.bits();
  int dl = dat.bytesPerLine() - 6*w;
  for (int y=0; y<hei; y++) {
    for (int x=0; x<wid; x++) {
      dst[0] = src[1];
      dst[1] = src[0];
      dst+=2;
      src+=2;
    }
    dst += dl;
  }
}
