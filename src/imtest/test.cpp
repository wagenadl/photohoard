// test.cpp

#include "Image.h"
#include <QDebug>
#include <QTime>
#include "../CMSProfile.h"
#include "../CMSTransform.h"

template <typename T> void report(T const *src, int N, int dN, double scl,
				  char const *lbl) {
  printf("%s:\n", lbl);
  for (int n=0; n<N; n++) {
    printf("  ");
    for (int c=0; c<3; c++) {
      double v = src[c + n*dN] / scl;
      printf("%10.3f", v);
      double v0 = fabs(v);
      double v1 = floor(v0*1000);
      printf(",%03.0f", (v0-v1/1000)*1e6);
    }
    printf("\n");
  }
}

int main(int argc, char **argv) {
  Image img("img.png");
  int N = img.width()*img.height();

  report((uchar const *)img.bits(), N, 4, 255, "sRGB");
  report((short const *)img
	 .convertedTo(Image::Format::Color14, Image::Space::LinearRGB).bits(),
	 N, 3, 16383, "RGB14");
  report((short const *)img
	 .convertedTo(Image::Format::Color14, Image::Space::XYZ).bits(),
	 N, 3, 16383, "XYZ14");
  report((short const *)img
	 .convertedTo(Image::Format::Color14, Image::Space::LabD50).bits(),
	 N, 3, 16383, "LabD50-14");
  Image lab14 = img.convertedTo(Image::Format::Color14, Image::Space::LabD50);
  report((uchar const *)lab14.convertedTo(Image::Format::Color8,
					  Image::Space::sRGB).bits(),
	 N, 4, 255, "<sRGB");
  printf("\n");

  report((uchar const *)img.bits(), N, 4, 255, "sRGB");
  
  short *buf1 = new short[N*3];
  short *buf2 = new short[N*3];

  //CMSProfile srgb("/usr/share/kde4/apps/libkdcraw/profiles/srgb-d65.icm");
  // "/home/opt/bibble5/supportfiles/Profiles/srgb.icm");
   CMSProfile srgb = CMSProfile::srgbProfile();
  CMSProfile rgb =  CMSProfile::linearRgbProfile();
  CMSProfile xyz = CMSProfile::xyzProfile();
  CMSProfile lab = CMSProfile::labProfile();
  CMSProfile lab65 = CMSProfile::labProfile(CMSProfile::StandardIlluminant::D65);

  CMSTransform xrgb(srgb, rgb,
		    CMSTransform::ImageFormat::uInt8_4,
		    CMSTransform::ImageFormat::uInt16,
		    CMSTransform::RenderingIntent::Perceptual);
  CMSTransform xxyz(srgb, xyz,
		    CMSTransform::ImageFormat::uInt8_4,
		    CMSTransform::ImageFormat::uInt16,
		    CMSTransform::RenderingIntent::Perceptual);
  CMSTransform xlab(srgb, lab,
		    CMSTransform::ImageFormat::uInt8_4,
		    CMSTransform::ImageFormat::uInt16,
		    CMSTransform::RenderingIntent::Perceptual);

  xrgb.apply(buf1, img.bits(), N);
  report((unsigned short *)buf1, N, 3, 65535, "RGB16");

  xxyz.apply(buf1, img.bits(), N);
  report((unsigned short *)buf1, N, 3, 32767, "XYZ16");

  xlab.apply(buf1, img.bits(), N);
  report((unsigned short *)buf1, N, 3, 655.35, "Lab16"); 

  CMSTransform xrev(lab65, srgb,
                      CMSTransform::ImageFormat::uInt16,
		    CMSTransform::ImageFormat::uInt8_4,
		    CMSTransform::RenderingIntent::AbsoluteColorimetric);
  CMSTransform xrev65(lab65, xyz,
                      CMSTransform::ImageFormat::uInt16,
		    CMSTransform::ImageFormat::uInt16,
		    CMSTransform::RenderingIntent::AbsoluteColorimetric);
  xrev65.apply(buf2, buf1, N);
  report((unsigned short *)buf2, N, 3, 32767, "XYZ16x");
  
  Image im2 = img;
  xrev.apply(im2.bits(), buf1, N);
  report(im2.bits(), N, 4, 255, "sRGB");

  printf("\n\n");
//////////////////////////////////////////////////////////////////////
  float *buf3 = new float[N*3];
  float *buf4 = new float[N*3];
  
  CMSTransform xrgb3(srgb, rgb,
		    CMSTransform::ImageFormat::uInt8_4,
		    CMSTransform::ImageFormat::Float,
		    CMSTransform::RenderingIntent::RelativeColorimetric);
  CMSTransform xxyz3(rgb, xyz,
		     CMSTransform::ImageFormat::Float,
		     CMSTransform::ImageFormat::Float,
		     CMSTransform::RenderingIntent::AbsoluteColorimetric,
		     false);
  CMSTransform xlab3(srgb, lab,
		     CMSTransform::ImageFormat::uInt8_4,
		     CMSTransform::ImageFormat::Float,
		     CMSTransform::RenderingIntent::RelativeColorimetric,
		     false);
  CMSTransform xlab4(xyz, lab,
		     CMSTransform::ImageFormat::Float,
		     CMSTransform::ImageFormat::Float,
		     CMSTransform::RenderingIntent::AbsoluteColorimetric,
		     false);

  xrgb3.apply(buf3, img.bits(), N);
  report(buf3, N, 3, 1., "RGB16");

  xxyz3.apply(buf4, buf3, N);
  report(buf4, N, 3, .5, "XYZ16");

  xlab3.apply(buf4, img.bits(), N);
  report(buf4, N, 3, 1., "Lab16"); 

  xlab4.apply(buf4, buf3, N);
  report(buf4, N, 3, 1., "Lab16x"); 

  return 0;
  
}

/* My test results show that lcms's uInt16 L*a*b* values are stored as:

     L*_int = (654.93 ± 0.10) L*_float +    29.7 ± 4.6
     a*_int = (257.00 ± 0.12) a*_float + 32894.9 ± 1.7
     b*_int = (256.86 ± 0.07) b*_float + 32899.4 ± 1.8

   This is, admittedly, a little peculiar. Also slightly disconcerting, since
   I thought I was doing the calculations correctly, lcms's uInt16 L*a*b*
   values are not a direct scale of my own "±1.14" values:

     L*_lcms = (4.0197 ± 0.0035) L*_dw +    96.6 ±  27.4
     a*_lcms = (1.6274 ± 0.0293) a*_dw + 33292.1 ±  62.9
     b*_lcms = (1.5417 ± 0.0487) b*_dw + 34897.7 ± 171.2

   This suggests that I may not be using the same illuminant.
*/
