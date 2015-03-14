// Clarity.cpp

#include "Clarity.h"
#include <QDebug>

namespace ClarityInternal {
#undef BYTE_IMAGE
#include "clahe.c"
};

bool zuiderveld_clahe(unsigned short *image,
                      int xblocksize, int xblockcount,
                      int yblocksize, int yblockcount, 
                      int nbins, double cliplimit) {

  qDebug() << "Starting CLAHE";
  int r = ClarityInternal::CLAHE(image,
                                 xblocksize*xblockcount, yblocksize*yblockcount,
                                 0, 65535,
                                 xblockcount, yblockcount,
                                 nbins, cliplimit);
  qDebug() << "CLAHE returned" << r;
  return r>=0;
}
