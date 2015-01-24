// test.cpp

#include "Image.h"
#include <QDebug>
#include <QTime>
#include "../CMSProfile.h"
#include "../CMSTransform.h"

template <typename T> void report(T const *src, int N, double scl,
				  char const *lbl) {
  printf("%s:\n", lbl);
  for (int n=0; n<N; n++) {
    printf("  ");
    for (int c=0; c<3; c++) {
      double v = src[c + N*3] / scl;
      printf("%10.6f", v);
    }
    printf("\n");
  }
}

int main(int argc, char **argv) {
  Image img("img.png");
  // ("/home/wagenaar/Pictures/Sam at San Rafael 130125.jpg");
  report((uchar const *)img.bits(), img.width(), 255, "sRGB8");

  Image srgb = img.convertedTo(Image::Format::Color14, Image::Space::Linear
  Image lab14 = img.convertedTo(Image::Format::Color14, Image::Space::LabD50);
  qDebug() << t0.elapsed();
  short *data = (short*)lab14.bits();
  int X = lab14.width();
  int Y = lab14.height();
  int dL = lab14.bytesPerLine() - 6*X;
  qDebug() << X << Y << dL;

  printf("dw = [\n");
  for (int n=1248; n<X*Y; n+=30*X+150) 
    printf("%i %i %i\n", data[3*n], data[3*n+1], data[3*n+2]);
  printf("];\n");
  t0.restart();
  for (int y=0; y<Y; y++) {
    for (int x=0; x<X; x++) {
      float v = *data / 16384.0f;
      v = pow(v, 0.8);
      *data = short(v * 16384.0f);
      data += 3;
    }
    data += dL;
  }
  qDebug() << t0.elapsed();

  qDebug() << int(lab14.format()) << int(lab14.space());;

  t0.restart();
  Image im1 = lab14.convertedToFormat(Image::Format::Color8);
  qDebug() << t0.elapsed();
  
  im1.save("/tmp/foo.jpg");
  #endif
  //////////////////////////////////////////////////////////////////////

  int N = img.width()*img.height();
  short *buf1 = new short[N*3];

  CMSProfile srgb = CMSProfile::srgbProfile();
  CMSProfile rgb =  CMSProfile::linearRgbProfile();
  CMSProfile lab = CMSProfile::labProfile();
  CMSProfile xyz = CMSProfile::xyzProfile();
  fprintf(stderr, "srgb: 0x%08x\n", srgb.signature());
  fprintf(stderr, "rgb:  0x%08x\n", rgb.signature());
  fprintf(stderr, "lab:  0x%08x\n", lab.signature());
  fprintf(stderr, "xyz:  0x%08x\n", xyz.signature());

  CMSTransform xform1(srgb, lab,
                      CMSTransform::ImageFormat::uInt8_4,
                      CMSTransform::ImageFormat::uInt16);
  t0.restart();
  xform1.apply(buf1, img.bits(), N);
  qDebug() << t0.elapsed();
  
  t0.restart();
  ushort *dat = (ushort*)buf1;
  printf("ui = [\n");
  for (int n=1248; n<X*Y; n+=30*X+150) 
    printf("%i %i %i\n", dat[3*n], dat[3*n+1], dat[3*n+2]);
  printf("];\n");
  for (int n=0; n<N; n++) {
    float v = *dat / 65535.0f;
    v = pow(v, 0.8);
    *dat = short(v * 65535.0f);
    dat += 3;
  }
  qDebug() << t0.elapsed();

  CMSTransform xform2(lab, srgb,
                      CMSTransform::ImageFormat::uInt16,
                      CMSTransform::ImageFormat::uInt8_4);

  Image im2 = img;
  t0.restart();
  xform2.apply(im2.bits(), buf1, N);
  qDebug() << t0.elapsed();
  
  im2.save("/tmp/foo1.jpg");

  qDebug() << "Saved";
  //////////////////////////////////////////////////////////////////////

  float *buf2 = new float[N*3];

  xform1 = CMSTransform(srgb, lab,
                      CMSTransform::ImageFormat::uInt8_4,
                      CMSTransform::ImageFormat::Float);
  t0.restart();
  xform1.apply(buf2, img.bits(), N);
  qDebug() << t0.elapsed();
  
  t0.restart();
  float *da = buf2;
  printf("fl = [\n");
  for (int n=1248; n<X*Y; n+=30*X+150) 
    printf("%g %g %g\n", da[3*n], da[3*n+1], da[3*n+2]);
  printf("];\n");
  for (int n=0; n<N; n++) {
    float v = *da / 100;
    v = pow(v, 0.8);
    *da = v * 100;
    da += 3;
  }
  qDebug() << t0.elapsed();

  xform2 = CMSTransform(lab, srgb,
                      CMSTransform::ImageFormat::Float,
                      CMSTransform::ImageFormat::uInt8_4);

  t0.restart();
  xform2.apply(img.bits(), buf2, N);
  qDebug() << t0.elapsed();

  img.save("/tmp/foo2.jpg");
  
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
