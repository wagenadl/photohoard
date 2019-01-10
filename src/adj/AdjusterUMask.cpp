// AdjusterUmask.cpp

#include "AdjusterUMask.h"
#include "Adjuster.h"
#include <math.h>

QStringList AdjusterUMask::fields() const {
  static QStringList flds
    = QString("umask umaskr")
    .split(" ");
  return flds;
}

class UMask {
public:
  UMask(double strength, double radius): strength(strength), radius(radius) { }
  void apply(Image16 &image, double nominalScale=1);
private:
  double strength;
  double radius;    
};

void UMask::apply(Image16 &image, double nominalScale) {
  double effrad = radius / nominalScale;

  // determine kernel size
  constexpr int maxhalfN = 3;
  int halfN = int(effrad*2.5+.49);
  if (halfN>maxhalfN)
    halfN=maxhalfN;
  else if (halfN<1)
    halfN=1;

  // calculate kernel
  float kernel[2*halfN+1];
  float *kptr = kernel+halfN;
  for (int n=-halfN; n<=halfN; n++) 
    kptr[n] = exp(-.5*n*n/(effrad*effrad));
  double sum = 0;
  for (int n=-halfN; n<=halfN; n++)
    sum += kptr[n];
  for (int n=-halfN; n<=halfN; n++)
    kptr[n] /= sum;

  const int width = image.width();
  const int height = image.height();
  QVector<float> filtered(width*height);
  // filter in x-direction
  // This could be multithreaded
  for (int y=0; y<height; y++) {
    quint16 const *pix = image.words() + image.wordsPerLine()*y;
    float *dst = filtered.data() + width*y;
    for (int x=0; x<halfN; x++) {
      *dst++ = *pix;
      pix += 3;
    }
    for (int x=halfN; x<width-halfN; x++) {
      float v = 0;
      for (int n=-halfN; n<=halfN; n++)
        v += kptr[n]*pix[3*n];
      *dst++ = v;
      pix += 3;
    }
    for (int x=0; x<halfN; x++) {
      *dst++ = *pix;
      pix += 3;
    }
  }
  // filter in y-direction and apply
  // we do not apply to edges
  int len = image.wordsPerLine();
  for (int x=halfN; x<width-halfN; x++) {
    quint16 *pix = image.words() + 3*x + len*halfN;
    float const *flt = filtered.data() + x + width*halfN;
    for (int y=halfN; y<height-halfN; y++) {
      float v = 0;
      for (int n=-halfN; n<=halfN; n++)
        v += kptr[n]*flt[width*n];
      float w = *pix;
      w += (w-v)*strength;
      *pix = (w<0) ? 0 : (w>65535) ? 65535 : quint16(w+.5);
      pix += len;
      flt += width;
    }
  }
}
  

AdjusterTile AdjusterUMask::apply(AdjusterTile const &parent,
				Adjustments const &fin) {
  AdjusterTile tile = parent;
  tile.stage = Stage_UMask;

  if (fin.umask) {
    tile.image.convertTo(Image16::Format::IPT16, maxthreads);
    UMask umask(fin.umask, fin.umaskr);
    umask.apply(tile.image, tile.nominalScale);
    // Do unsharp mask only on I channel?
    // Alternatively, I could work in LMS space and treat all channels
  }
  
  tile.settings.umask = fin.umask;
  tile.settings.umaskr = fin.umaskr;

  return tile;
}
