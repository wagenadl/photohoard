// CS_Lab.cpp

#include "ColorSpaces.h"
#include <math.h>

namespace ColorSpaces {
  uint16_t LabFwd[lab_tablesize];
  uint16_t LabRev[lab_tablesize];

  class LabBuilder {
  public:
    LabBuilder() {
      for (int k=0; k<32768; k++) {
        double t = k/32767.0;
        t = (t>(6*6*6)/(29*29*29.0)) ? pow(t, 1/3.0)
          : (29*29)/(3*6*6.0)*t + 4/29.0;
        LabFwd[k] = t*32767.499 + .5;
      }

      for (int k=0; k<32768; k++) {
        double t = k/32767.0;
        t = (t>6/29.0) ? pow(t, 3.0) : (3*6*6.0)/(29*29)*(t - 4/29.0);
        if (t<0)
          t = 0;
        LabRev[k] = t*32767.499 + .5;
      }
    }
  };

  static LabBuilder labbuilder;
}
