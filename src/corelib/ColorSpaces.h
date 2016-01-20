// ColorSpaces.h

#ifndef COLORSPACES_H

#define COLORSPACES_H

#include <stdint.h>
#include "PDebug.h"
#include <stdlib.h>

namespace ColorSpaces {

  typedef uint16_t uint15_t;

  struct XYZ {
    uint15_t X, Y, Z;
  };

  struct Lab {
    uint16_t L; // 655.35×L* 
    int16_t a, b; // 256×a*; 256×b*
  };

  struct XYZp { // XYZ but scaled so that D65 whitepoint is (1,1,1).
    uint15_t Xp, Y, Zp;
  };

  struct sRGB {
    uint8_t B, G, R, A;
  };
  
  struct IPT {
    uint16_t I;
    int16_t P, T;
  };

  struct LMS {
    uint15_t L, M, S;
  };

  constexpr uint32_t xyz_logscale = 15;
  constexpr uint32_t xyz_scale = 1 << xyz_logscale;

  constexpr double d65_X = 0.9505;
  constexpr double d65_Z = 1.0890;

  template <typename SRC, typename DST>
  inline void convert(SRC const &src, DST &dst) {
    XYZ tmp;
    //    qDebug() << "Indirect";
    convert(src, tmp);
    convert(tmp, dst);
  }

  template <> inline void convert(XYZ const &, XYZ &) {
    pDebug() << "Missing converter";
    exit(1);
  }

  #include "CS_sRGB.h"
  #include "CS_Lab.h"
  #include "CS_IPT.h"

  template <> inline void convert(sRGB const &src, Lab &dst) {
    XYZp tmp;
    convert(src, tmp);
    convert(tmp, dst);
  }

  template <> inline void convert(Lab const &src, sRGB &dst) {
    XYZp tmp;
    convert(src, tmp);
    convert(tmp, dst);
  }
  
  template <typename SRC, typename DST>
  inline void convertImage(SRC const *src, int W, int H, int SB,
                           DST *dst, int DB) {
    //    qDebug() << src << W << H << SB << dst << DB << sizeof(SRC) << sizeof(DST);
    int sb = SB - W*sizeof(SRC);
    int db = DB - W*sizeof(DST);
    for (int y=0; y<H; y++) {
      for (int x=0; x<W; x++) 
        convert(*src++, *dst++);
      src = (SRC const *)((uint8_t const*)src + sb);
      dst = (DST *)((uint8_t *)dst + db);
    }
  }
};  

#endif
