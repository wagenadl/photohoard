// CS_sRGB.h

#ifndef CS_SRGB_H

#define CS_SRGB_H

extern uint16_t sRGB2XYZ[3][256][3];
extern uint16_t sRGB2XYZp[3][256][2];
extern uint16_t sRGB2LMS[3][256][3];
constexpr int32_t srgb_tablesize = 16384;
extern uint8_t RGB2sRGB[srgb_tablesize];

constexpr double r2x = .4124;
constexpr double r2y = .2126;
constexpr double r2z = .0193;
constexpr double g2x = .3576;
constexpr double g2y = .7152;
constexpr double g2z = .1192;
constexpr double b2x = .1805;
constexpr double b2y = .0722;
constexpr double b2z = .9505;

constexpr double x2r = 3.2406;
constexpr double y2r = -1.5372;
constexpr double z2r = -0.4986;
constexpr double x2g = -0.9689;
constexpr double y2g = 1.8758;
constexpr double z2g = 0.0415;
constexpr double x2b = 0.0557;
constexpr double y2b = -0.2040;
constexpr double z2b = 1.0570;

constexpr double r2l = 0.313899;
constexpr double r2m = 0.151644;
constexpr double r2s = 0.017725;
constexpr double g2l = 0.639496;
constexpr double g2m = 0.748242;
constexpr double g2s = 0.109473;
constexpr double b2l = 0.046612;
constexpr double b2m = 0.100047;
constexpr double b2s = 0.872939;

constexpr double l2r =  5.432049;
constexpr double l2g = -1.104672;
constexpr double l2b =  0.028236;
constexpr double m2r = -4.678594;
constexpr double m2g =  2.310706;
constexpr double m2b = -0.194781;
constexpr double s2r =  0.246154;
constexpr double s2g = -0.205841;
constexpr double s2b =  1.166371;


template <> inline void convert<sRGB, XYZ>(sRGB const &src, XYZ &dst) {
  uint8_t b = src.B;
  uint8_t g = src.G;
  uint8_t r = src.R;
  dst.X = sRGB2XYZ[0][r][0] + sRGB2XYZ[1][g][0] + sRGB2XYZ[2][b][0];
  dst.Y = sRGB2XYZ[0][r][1] + sRGB2XYZ[1][g][1] + sRGB2XYZ[2][b][1];
  dst.Z = sRGB2XYZ[0][r][2] + sRGB2XYZ[1][g][2] + sRGB2XYZ[2][b][2];
}

template <> inline void convert<sRGB, XYZp>(sRGB const &src, XYZp &dst) {
  uint8_t b = src.B;
  uint8_t g = src.G;
  uint8_t r = src.R;
  dst.Xp = sRGB2XYZp[0][r][0] + sRGB2XYZp[1][g][0] + sRGB2XYZp[2][b][0];
  dst.Y =  sRGB2XYZ[0][r][1]  + sRGB2XYZ[1][g][1]  + sRGB2XYZ[2][b][1];
  dst.Zp = sRGB2XYZp[0][r][1] + sRGB2XYZp[1][g][1] + sRGB2XYZp[2][b][1];
}

template <> inline void convert<sRGB, LMS>(sRGB const &src, LMS &dst) {
  uint8_t b = src.B;
  uint8_t g = src.G;
  uint8_t r = src.R;
  dst.L = sRGB2LMS[0][r][0] + sRGB2LMS[1][g][0] + sRGB2LMS[2][b][0];
  dst.M = sRGB2LMS[0][r][1] + sRGB2LMS[1][g][1] + sRGB2LMS[2][b][1];
  dst.S = sRGB2LMS[0][r][2] + sRGB2LMS[1][g][2] + sRGB2LMS[2][b][2];
}

template <> inline void convert<XYZ, sRGB>(XYZ const &src, sRGB &dst) {
  constexpr int32_t x2ri = int32_t(x2r*srgb_tablesize + 0.5);
  constexpr int32_t y2ri = int32_t(y2r*srgb_tablesize + 0.5);
  constexpr int32_t z2ri = int32_t(z2r*srgb_tablesize + 0.5);
  constexpr int32_t x2gi = int32_t(x2g*srgb_tablesize + 0.5);
  constexpr int32_t y2gi = int32_t(y2g*srgb_tablesize + 0.5);
  constexpr int32_t z2gi = int32_t(z2g*srgb_tablesize + 0.5);
  constexpr int32_t x2bi = int32_t(x2b*srgb_tablesize + 0.5);
  constexpr int32_t y2bi = int32_t(y2b*srgb_tablesize + 0.5);
  constexpr int32_t z2bi = int32_t(z2b*srgb_tablesize + 0.5);

  constexpr int32_t thresh = xyz_scale * srgb_tablesize;
  
  int32_t x = src.X;
  int32_t y = src.Y;
  int32_t z = src.Z;
  int32_t r = x2ri*x + y2ri*y + z2ri*z;
  int32_t g = x2gi*x + y2gi*y + z2gi*z;
  int32_t b = x2bi*x + y2bi*y + z2bi*z;
  uint16_t R = (r<0) ? 0 : (r>=thresh) ? srgb_tablesize-1 : (r>>xyz_logscale);
  uint16_t G = (g<0) ? 0 : (g>=thresh) ? srgb_tablesize-1 : (g>>xyz_logscale);
  uint16_t B = (b<0) ? 0 : (b>=thresh) ? srgb_tablesize-1 : (b>>xyz_logscale);
  dst.B = RGB2sRGB[B];
  dst.G = RGB2sRGB[G];
  dst.R = RGB2sRGB[R];
  dst.A = 255;
}

template <> inline void convert<XYZp, sRGB>(XYZp const &src, sRGB &dst) {
  constexpr int32_t x2ri = int32_t(x2r*d65_X*srgb_tablesize + 0.5);
  constexpr int32_t y2ri = int32_t(y2r*srgb_tablesize + 0.5);
  constexpr int32_t z2ri = int32_t(z2r*d65_Z*srgb_tablesize + 0.5);
  constexpr int32_t x2gi = int32_t(x2g*d65_X*srgb_tablesize + 0.5);
  constexpr int32_t y2gi = int32_t(y2g*srgb_tablesize + 0.5);
  constexpr int32_t z2gi = int32_t(z2g*d65_Z*srgb_tablesize + 0.5);
  constexpr int32_t x2bi = int32_t(x2b*d65_X*srgb_tablesize + 0.5);
  constexpr int32_t y2bi = int32_t(y2b*srgb_tablesize + 0.5);
  constexpr int32_t z2bi = int32_t(z2b*d65_Z*srgb_tablesize + 0.5);

  constexpr int32_t thresh = xyz_scale * srgb_tablesize;
  
  int32_t x = src.Xp;
  int32_t y = src.Y;
  int32_t z = src.Zp;
  int32_t r = x2ri*x + y2ri*y + z2ri*z;
  int32_t g = x2gi*x + y2gi*y + z2gi*z;
  int32_t b = x2bi*x + y2bi*y + z2bi*z;
  uint16_t R = (r<0) ? 0 : (r>=thresh) ? srgb_tablesize-1 : (r>>xyz_logscale);
  uint16_t G = (g<0) ? 0 : (g>=thresh) ? srgb_tablesize-1 : (g>>xyz_logscale);
  uint16_t B = (b<0) ? 0 : (b>=thresh) ? srgb_tablesize-1 : (b>>xyz_logscale);
  dst.B = RGB2sRGB[B];
  dst.G = RGB2sRGB[G];
  dst.R = RGB2sRGB[R];
  dst.A = 255;
}

template <> inline void convert<LMS, sRGB>(LMS const &src, sRGB &dst) {
  constexpr int32_t l2ri = int32_t(l2r*srgb_tablesize + 0.5);
  constexpr int32_t m2ri = int32_t(m2r*srgb_tablesize + 0.5);
  constexpr int32_t s2ri = int32_t(s2r*srgb_tablesize + 0.5);
  constexpr int32_t l2gi = int32_t(l2g*srgb_tablesize + 0.5);
  constexpr int32_t m2gi = int32_t(m2g*srgb_tablesize + 0.5);
  constexpr int32_t s2gi = int32_t(s2g*srgb_tablesize + 0.5);
  constexpr int32_t l2bi = int32_t(l2b*srgb_tablesize + 0.5);
  constexpr int32_t m2bi = int32_t(m2b*srgb_tablesize + 0.5);
  constexpr int32_t s2bi = int32_t(s2b*srgb_tablesize + 0.5);

  constexpr int32_t thresh = xyz_scale * srgb_tablesize;
  
  int32_t l = src.L;
  int32_t m = src.M;
  int32_t s = src.S;
  int32_t r = l2ri*l + m2ri*m + s2ri*s;
  int32_t g = l2gi*l + m2gi*m + s2gi*s;
  int32_t b = l2bi*l + m2bi*m + s2bi*s;
  uint16_t R = (r<0) ? 0 : (r>=thresh) ? srgb_tablesize-1 : (r>>xyz_logscale);
  uint16_t G = (g<0) ? 0 : (g>=thresh) ? srgb_tablesize-1 : (g>>xyz_logscale);
  uint16_t B = (b<0) ? 0 : (b>=thresh) ? srgb_tablesize-1 : (b>>xyz_logscale);
  dst.B = RGB2sRGB[B];
  dst.G = RGB2sRGB[G];
  dst.R = RGB2sRGB[R];
  dst.A = 255;
}


#endif
