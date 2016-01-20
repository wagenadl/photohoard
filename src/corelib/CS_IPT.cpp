// CS_IPT.cpp

#include "ColorSpaces.h"
#include <math.h>

namespace ColorSpaces {
  
  uint16_t LMS2IPT[32768];
  uint16_t IPT2LMS[32768];

  class IPTBuilder {
  public:
    IPTBuilder() {
      for (int k=0; k<32768; k++) {
        double lms = k/32767.0;
        LMS2IPT[k] = uint16_t(32768*pow(lms, 0.43) + 0.5);
      }

      for (int k=0; k<32768; k++) {
        double ipt = k/32767.0;
        IPT2LMS[k] = uint16_t(32768*pow(ipt, 1/0.43) + 0.5);
      }
    }
  };

  static IPTBuilder iptbuilder;
}
      
      
