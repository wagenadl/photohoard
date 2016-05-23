// AdjusterEqualize.cpp

#include "AdjusterEqualize.h"
#include "Adjuster.h"

QStringList AdjusterEqualize::fields() const {
  static QStringList flds
    = QString("nlcontrast nlcontrastscale")
    .split(" ");
  return flds;
}

#if ENABLE_CLARITY

#include "Clarity.h"

static void applyClarity(Image16 &target, double clarity, int n) {

  // Do clarity only on I channel
  const int longblocks = n; // number of blocks in long dimension of image
  const int width = target.width();
  const int height = target.height();
  const int blocksize = (target.size().maxDim()+longblocks-1) / longblocks;
  const int xblockcount = (width+blocksize-1) / blocksize;
  const int yblockcount = (height+blocksize-1) / blocksize;
  const int xmargin = xblockcount*blocksize - width;
  const int ymargin = yblockcount*blocksize - height;
  const int leftmargin = xmargin/2;
  const int rightmargin = xmargin - leftmargin;
  const int topmargin = ymargin/2;
  const int bottommargin = ymargin - topmargin;

  if (xblockcount<2 || yblockcount<2) {
    COMPLAIN("Cannot apply CLAHE with fewer than 2x2 blocks");
    return;
  }
  if (leftmargin>width || rightmargin>width || topmargin>height || bottommargin>height) {
    COMPLAIN("Cannot apply CLAHE on tiny images. Try increasing block count");
    return;
  }

  QVector<quint16> vec(blocksize*blocksize*xblockcount*yblockcount);
  quint16 *image = vec.data();

  // Copy defined part of image
  for (int y=0; y<height; y++) {
    quint16 const *src = target.words() + y*target.wordsPerLine();
    quint16 *dst = image + (y+topmargin)*(blocksize*xblockcount)+leftmargin;
    for (int x=0; x<width; x++) {
      *dst++ = *src;
      src += 3;
    }
  }

  // Fill in left/right margins
  for (int y=0; y<height; y++) {
    quint16 *dst = image + (y+topmargin)*(blocksize*xblockcount)+leftmargin;
    for (int x=0; x<leftmargin; x++)
      dst[-x-1] = dst[x];
    dst = image + (y+ymargin)*(blocksize*xblockcount)+leftmargin+width;
    for (int x=0; x<rightmargin; x++)
      dst[x] = dst[-x-1];
  }

  // Fill in top/bottom margins
  for (int y=0; y<topmargin; y++) {
    quint16 *dst = image + (topmargin-y-1)*(blocksize*xblockcount);
    quint16 const *src = image + (topmargin+y)*(blocksize*xblockcount);
    memcpy(dst, src, 2*blocksize*xblockcount);
  }
  for (int y=0; y<bottommargin; y++) {
    quint16 *dst = image + (topmargin+height+y)*(blocksize*xblockcount);
    quint16 const *src = image + (topmargin+height-y-1)
      *(blocksize*xblockcount);
    memcpy(dst, src, 2*blocksize*xblockcount);
  }

  bool ok = zuiderveld_clahe(image,
                             blocksize, xblockcount,
                             blocksize, yblockcount,
                             4096, exp(clarity));
  if (!ok) {
    COMPLAIN("CLAHE failed");
    return;
  }

  // Copy defined part of image back
  for (int y=0; y<height; y++) {
    quint16 *dst = target.words() + y*target.wordsPerLine();
    quint16 const *src
      = image + (y+topmargin)*(blocksize*xblockcount)+leftmargin;
    for (int x=0; x<width; x++) {
      *dst = *src++;
      dst += 3;
    }
  }  
}

#endif

static float getScale(int maxdim, float scale) {
  float f = pow(1.0/maxdim, 1-scale);
  return f;
}

static void applyLocalContrast(Image16 &target, double contrast, double scale,
                               int /*maxthreads*/) {
  const int width = target.width();
  const int height = target.height();
  const int L = width*height;
  QVector<float> img(L);
  float *image = img.data();
  // Copy defined part of image
  for (int y=0; y<height; y++) {
    quint16 const *src = target.words() + y*target.wordsPerLine();
    float *dst = image + y*width;
    for (int x=0; x<width; x++) {
      *dst++ = *src;
      src += 3;
    }
  }

  // Filter down several times in each direction
  const float f0 = getScale(target.size().maxDim(), scale);
  const float contr = contrast/3.0;
  const int N = 3;

  // This *could* be multithreaded...
  // horizontal
  float z = image[width-1];
  float *ptr = image + width-1;
  for (int x=1; x<width; x++) { // first prep z
    ptr--;
    z += f0*(*ptr-z);
  }
  for (int y=0; y<height; y++) {
    ptr = image + y*width;
    for (int n=0; n<N; n++) {
      for (int x=1; x<width; x++) {
        ptr++;
        z += f0*(*ptr-z);
        *ptr = z;
      }
      for (int x=1; x<width; x++) {
        ptr--;
        z += f0*(*ptr-z);
        *ptr = z;
      }
    }
  }

  // vertical round
  z = image[(height-1)*width];
  ptr = image + (height-1)*width;
  for (int y=1; y<height; y++) { // first prep z
    ptr-=width;
    z += f0*(*ptr-z);
  }
  for (int x=0; x<width; x++) {
    ptr = image + x;
    for (int n=0; n<N; n++) {
      for (int y=1; y<height; y++) {
        ptr += width;
        z += f0*(*ptr-z);
        *ptr = z;
      }
      for (int y=1; y<height; y++) {
        ptr -= width;
        z += f0*(*ptr-z);
        *ptr = z;
      }
    }
  }

  for (int y=0; y<height; y++) {
    quint16 *dst =  target.words() + y*target.wordsPerLine();
    float const *src = image + y*width;
    for (int x=0; x<width; x++) {
      float z = *dst;
      z += contr*(z-*src);
      *dst = (z>65535) ? 65535 : (z<0) ? 0 : z + 0.5;
      src ++;
      dst += 3;
    }
  }
}


AdjusterTile AdjusterEqualize::apply(AdjusterTile const &parent,
				Adjustments const &final) {
  AdjusterTile tile = parent;
  tile.stage = Stage_Equalize;

  if (final.nlcontrast) {
    tile.image.convertTo(Image16::Format::IPT16, maxthreads);
    applyLocalContrast(tile.image, final.nlcontrast, final.nlcontrastscale,
                       maxthreads);
  }

  tile.settings.nlcontrast = final.nlcontrast;
  tile.settings.nlcontrastscale = final.nlcontrastscale;
  
  return tile;
}
