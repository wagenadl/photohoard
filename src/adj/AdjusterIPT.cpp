// AdjusterIPT.cpp

#include "AdjusterIPT.h"

QStringList AdjusterIPT::fields() const {
  static QStringList flds
    = QString("shadows midtones highlights saturation vibrance")
    .split(" ");
  return flds;
}

static inline double foo_shadow(double x, double a) {
  return a*x / (1+(a-1)*(1-(1-x)*(1-x)));
}

static inline double foo_highlight(double x, double a) {
  return 1 - foo_shadow(1-x, 1/a);
}

static inline double foo_midtone(double x, double a) {
  return a*x/(1+(a-1)*(1-(1-x)));
}

static inline double foo_saturation(double x, double a) {
  if (a>=1)
    return a*x / (1 + (a-1)*pow(x, a/(a-1)));
  else
    return a*x;
}

static inline double foo_vibrance(double x, double a) {
  return a*x / (1+(a-1)*(1-pow(1-x, 7)));
}
  

AdjusterTile AdjusterIPT::apply(AdjusterTile const &parent,
				Adjustments const &final) {
  AdjusterTile tile = parent;
  
  tile.stage = Stage_IPT;
  tile.image.convertTo(Image16::Format::IPT16);

  double shadows = pow(2, final.shadows);
  double highlights = pow(2, final.highlights);
  double midtones = pow(2, final.midtones);
  quint16 ilut[65536];
  for (int k=0; k<65536; k++) {
    double x = k/65535.0;
    x = foo_midtone(x, midtones);
    x = foo_shadow(x, shadows);
    x = foo_highlight(x, highlights);
    ilut[k] = (x<0) ? 0 : (x>=1.) ? 65535 : quint16(x*65535.499 + 0.5);
  }
  
  quint16 *words = tile.image.words();
  int X = tile.image.width();
  int Y = tile.image.height();
  int DL = tile.image.wordsPerLine() - 3*X;
  for (int y=0; y<Y; y++) {
    for (int x=0; x<X; x++) {
      *words = ilut[*words];
      words+=3;
    }
    words += DL;
  }

  if (final.saturation!=0 || final.vibrance!=0) {
    double sat = pow(2, final.saturation);
    double vib = pow(2, final.vibrance);
    constexpr int maxk = 32768*1.4142;
    constexpr int maxfac = 100;
    constexpr int scale = 32768;
    float clut[maxk];
    clut[0] = scale;
    for (int k=1; k<maxk; k++) {
      double x = k/32767.0;
      if (x>1)
        x = 1;
      double y = foo_vibrance(x, vib);
      y = foo_saturation(y, sat);
      double fac = y/x;
      clut[k] = scale * ((fac<maxfac) ? fac : maxfac);
    }
    qint16 *words = (qint16*)tile.image.words();
    int X = tile.image.width();
    int Y = tile.image.height();
    int DL = tile.image.wordsPerLine() - 3*X;
    for (int y=0; y<Y; y++) {
      for (int x=0; x<X; x++) {
        int p = words[1];
        int t = words[2];
        int p2_ = p*p;
        int t2_ = t*t;
        float p2 = p2_;
        float t2 = t2_;
        int c = sqrtf(p2+t2);
        int fac = clut[c];
        p *= fac;
        p /= scale;
        t *= fac;
        t /= scale;
        words[1] = (p<-32767) ? -32767 : (p>32767) ? 32767 : p;
        words[2] = (t<-32767) ? -32767 : (t>32767) ? 32767 : t;
        words += 3;
      }
      words += DL;
    }
  }
  
  tile.settings.shadows = final.shadows;
  tile.settings.highlights = final.highlights;
  tile.settings.midtones = final.midtones;
  tile.settings.saturation = final.saturation;
  tile.settings.vibrance = final.vibrance;

  return tile;
}
