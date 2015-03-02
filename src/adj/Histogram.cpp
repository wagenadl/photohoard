// Histogram.cpp

#include "Histogram.h"

Histogram::Histogram() {
  nbins = nextbins = 256;
  fmt = nextfmt = Image16::Format::sRGB8;
}

void Histogram::setImage(Image16 const &img) {
  image = img;
  for (int k=0; k<3; k++)
    hst[k].clear();
}

void Histogram::setNumberOfBins(int n) {
  nextbins = n;
}

void Histogram::setColorSpace(Image16::Format f) {
  nextfmt = f;
}

QVector<double> const &Histogram::channel(int k) const {
  static QVector<double> none;
  if (k<0 || k>2)
    return none;
  if (!isInstantiated())
    recalculate();
  return hst[k];
}

int Histogram::numberOfBins() const {
  return isInstantiated() ? nbins : nextbins;
}

Image16::Format Histogram::colorSpace() const {
  return isInstantiated() ? fmt : nextfmt;
}

bool Histogram::isInstantiated() const {
  return !hst[0].isEmpty();
}

void Histogram::recalculate() const {
  /* We only recalculate when needed. This is why hst is declared mutable. */

  fmt = nextfmt;
  nbins = nextbins;

  for (int k=0; k<3; k++) {
    hst[k].resize(nbins);
    for (int n=0; n<nbins; n++)
      hst[k][n] = 0;
  }

  image.convertTo(fmt);
  int X = image.width();
  int Y = image.height();

  if (X==0 || Y==0)
    return; // empty image has no histo.
  
  int L = image.bytesPerLine();
  const Image16 &cimg = image;

  switch (fmt) {
  case Image16::Format::sRGB8: {
    // 8 bit calculations
    int DL = L - 4*X;
    uchar const *src = cimg.bytes();
    for (int y=0; y<Y; y++) {
      for (int x=0; x<X; x++) {
        for (int k=0; k<3; k++) {
          int v = *src++;
          int bin = v*nbins/256;
          hst[k][bin]++;
        }
        src++;
      }
      src += DL;
    }
  } break;
  default: {
    // 16 bit calculations
    int DL = L - 3*X;
    quint16 const *src = cimg.words();
    for (int y=0; y<Y; y++) {
      for (int x=0; x<X; x++) {
        for (int k=0; k<3; k++) {
          int v = *src++;
          int bin = v*nbins/65536;
          hst[k][bin]++;
        }
      }
      src += DL;
    }
  } break;
  }
  for (int k=0; k<3; k++)
    for (int n=0; n<nbins; n++)
      hst[k][n] /= X*Y;

  image = Image16();
}
