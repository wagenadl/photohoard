// CS_sRGB.cpp

#include "ColorSpaces.h"
#include <math.h>

namespace ColorSpaces {

  uint16_t sRGB2XYZ[3][256][3];
  uint16_t sRGB2XYZp[3][256][2];
  uint16_t sRGB2LMS[3][256][3];
  uint8_t RGB2sRGB[srgb_tablesize];

  class RGBBuilder {
  public:
    RGBBuilder() {
      for (int k=0; k<256; k++) {
        double srgb = k/255.0;
        double rgb = (srgb<.04045) ? (srgb/12.92) : pow((srgb+.055)/1.055, 2.4);
        sRGB2XYZ[0][k][0] = uint16_t(r2x*xyz_scale*rgb + 0.5);
        sRGB2XYZ[0][k][1] = uint16_t(r2y*xyz_scale*rgb + 0.5);
        sRGB2XYZ[0][k][2] = uint16_t(r2z*xyz_scale*rgb + 0.5);
        sRGB2XYZ[1][k][0] = uint16_t(g2x*xyz_scale*rgb + 0.5);
        sRGB2XYZ[1][k][1] = uint16_t(g2y*xyz_scale*rgb + 0.5);
        sRGB2XYZ[1][k][2] = uint16_t(g2z*xyz_scale*rgb + 0.5);
        sRGB2XYZ[2][k][0] = uint16_t(b2x*xyz_scale*rgb + 0.5);
        sRGB2XYZ[2][k][1] = uint16_t(b2y*xyz_scale*rgb + 0.5);
        sRGB2XYZ[2][k][2] = uint16_t(b2z*xyz_scale*rgb + 0.5);

	sRGB2XYZp[0][k][0] = uint16_t(r2x/d65_X*xyz_scale*rgb + 0.5);
        sRGB2XYZp[0][k][1] = uint16_t(r2z/d65_Z*xyz_scale*rgb + 0.5);
        sRGB2XYZp[1][k][0] = uint16_t(g2x/d65_X*xyz_scale*rgb + 0.5);
        sRGB2XYZp[1][k][1] = uint16_t(g2z/d65_Z*xyz_scale*rgb + 0.5);
        sRGB2XYZp[2][k][0] = uint16_t(b2x/d65_X*xyz_scale*rgb + 0.5);
        sRGB2XYZp[2][k][1] = uint16_t(b2z/d65_Z*xyz_scale*rgb + 0.5);

        sRGB2LMS[0][k][0] = uint16_t(r2l*xyz_scale*rgb + 0.5);
        sRGB2LMS[0][k][1] = uint16_t(r2m*xyz_scale*rgb + 0.5);
        sRGB2LMS[0][k][2] = uint16_t(r2s*xyz_scale*rgb + 0.5);
        sRGB2LMS[1][k][0] = uint16_t(g2l*xyz_scale*rgb + 0.5);
        sRGB2LMS[1][k][1] = uint16_t(g2m*xyz_scale*rgb + 0.5);
        sRGB2LMS[1][k][2] = uint16_t(g2s*xyz_scale*rgb + 0.5);
        sRGB2LMS[2][k][0] = uint16_t(b2l*xyz_scale*rgb + 0.5);
        sRGB2LMS[2][k][1] = uint16_t(b2m*xyz_scale*rgb + 0.5);
        sRGB2LMS[2][k][2] = uint16_t(b2s*xyz_scale*rgb + 0.5);
      }

      for (int k=0; k<srgb_tablesize; k++) {
        double rgb = k/(srgb_tablesize - 1.0);
        double srgb = rgb<.0031308 ? (12.92*rgb) : 1.055*pow(rgb, 1/2.4) - .055;
        RGB2sRGB[k] = uint8_t(255.999*srgb);
      }
    }
  };
  static RGBBuilder rgbbuilder;
  
}

    
