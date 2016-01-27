// Clarity.cpp

#include "Clarity.h"
#include "PDebug.h"

namespace ClarityInternal {
#undef BYTE_IMAGE
#include "clahe.c"
};

bool zuiderveld_clahe(unsigned short *image,
                      int xblocksize, int xblockcount,
                      int yblocksize, int yblockcount, 
                      int nbins, double cliplimit) {

  int r = ClarityInternal::CLAHE(image,
                                 xblocksize*xblockcount, yblocksize*yblockcount,
                                 0, 65535,
                                 xblockcount, yblockcount,
                                 nbins, cliplimit);
  return r>=0;
}
