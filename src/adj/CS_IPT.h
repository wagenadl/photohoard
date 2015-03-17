// CS_IPT.h

#ifndef CS_IPT_H

#define CS_IPT_H

extern uint16_t LMS2IPT[32768];
extern uint16_t IPT2LMS[32768];

constexpr double x2l = 0.4002;
constexpr double x2m = -.2280;
constexpr double x2s = 0;
constexpr double y2l = 0.7075;
constexpr double y2m = 1.1500;
constexpr double y2s = 0;
constexpr double z2l = -.0807;
constexpr double z2m = 0.0612;
constexpr double z2s = 0.9184;

constexpr double l2x =  1.85024;
constexpr double l2y =  0.36683;
constexpr double l2z =  0.00000;
constexpr double m2x = -1.13830;
constexpr double m2y =  0.64388;
constexpr double m2z =  0.00000;
constexpr double s2x =  0.23843;
constexpr double s2y = -0.01067;
constexpr double s2z =  1.08885;


constexpr double l2i =  0.40000;
constexpr double l2p =  4.45500;
constexpr double l2t =  0.80560;
constexpr double m2i =  0.40000;
constexpr double m2p = -4.85100;
constexpr double m2t =  0.35720;
constexpr double s2i =  0.20000;
constexpr double s2p =  0.39600;
constexpr double s2t = -1.16280;

constexpr double i2l =  1.000000;
constexpr double i2m =  1.000000;
constexpr double i2s =  1.000000;
constexpr double p2l =  0.097569;
constexpr double p2m = -0.113876;
constexpr double p2s =  0.032615;
constexpr double t2l =  0.205226;
constexpr double t2m =  0.133217;
constexpr double t2s = -0.676887;


/* Although in the definition of Febner, LMS is not positive-bounded,
   any value within sRGB gamut _is_ within [0,1] in LMS.
   Therefore, I will force my LMS values to be nonnegative.
   For now, I will not force LMS values to be <=1.
   This is an arbitrary decision; out-of-gammut mapping is a tricky issue.
*/

template <> inline void convert<XYZ, LMS>(XYZ const &src, LMS &dst) {
  constexpr int32_t scale = 16384;
  constexpr int32_t scalebits = 14;
  constexpr int32_t halfscale = 8191;
  constexpr int32_t x2li = int32_t(x2l*scale + 0.5);
  constexpr int32_t x2mi = int32_t(x2m*scale + 0.5);
  constexpr int32_t x2si = int32_t(x2s*scale + 0.5);
  constexpr int32_t y2li = int32_t(y2l*scale + 0.5);
  constexpr int32_t y2mi = int32_t(y2m*scale + 0.5);
  constexpr int32_t y2si = int32_t(y2s*scale + 0.5);
  constexpr int32_t z2li = int32_t(z2l*scale + 0.5);
  constexpr int32_t z2mi = int32_t(z2m*scale + 0.5);
  constexpr int32_t z2si = int32_t(z2s*scale + 0.5);
  int32_t x = src.X;
  int32_t y = src.Y;
  int32_t z = src.Z;
  int32_t l = x2li*x + y2li*y + z2li*z + halfscale;
  int32_t m = x2mi*x + y2mi*y + z2mi*z + halfscale;
  int32_t s = x2si*x + y2si*y + z2si*z + halfscale;
  dst.L = (l<=0) ? 0 : (l>>scalebits);
  dst.M = (m<=0) ? 0 : (m>>scalebits);
  dst.S = (s<=0) ? 0 : (s>>scalebits);
}

template <> inline void convert<LMS, XYZ>(LMS const &src, XYZ &dst) {
  constexpr int32_t scale = 16384;
  constexpr int32_t scalebits = 14;
  constexpr int32_t halfscale = 8191;
  constexpr int32_t l2xi = int32_t(l2x*scale + 0.5);
  constexpr int32_t l2yi = int32_t(l2y*scale + 0.5);
  constexpr int32_t l2zi = int32_t(l2z*scale + 0.5);
  constexpr int32_t m2xi = int32_t(m2x*scale + 0.5);
  constexpr int32_t m2yi = int32_t(m2y*scale + 0.5);
  constexpr int32_t m2zi = int32_t(m2z*scale + 0.5);
  constexpr int32_t s2xi = int32_t(s2x*scale + 0.5);
  constexpr int32_t s2yi = int32_t(s2y*scale + 0.5);
  constexpr int32_t s2zi = int32_t(s2z*scale + 0.5);
  int32_t l = src.L;
  int32_t m = src.M;
  int32_t s = src.S;
  int32_t x = l2xi*l + m2xi*m + s2xi*s + halfscale;
  int32_t y = l2yi*l + m2yi*m + s2yi*s + halfscale;
  int32_t z = l2zi*l + m2zi*m + s2zi*s + halfscale;
  dst.X = (x<=0) ? 0 : (x>>scalebits);
  dst.Y = (y<=0) ? 0 : (y>>scalebits);
  dst.Z = (z<=0) ? 0 : (z>>scalebits);
}

template <> inline void convert<LMS, IPT>(LMS const &src, IPT &dst) {
  constexpr int32_t logscale = 15;
  constexpr int32_t scale = (1<<logscale);
  constexpr int32_t l2ii = int32_t(l2i*scale + 0.5);
  constexpr int32_t l2pi = int32_t(l2p*scale + 0.5);
  constexpr int32_t l2ti = int32_t(l2t*scale + 0.5);
  constexpr int32_t m2ii = int32_t(m2i*scale + 0.5);
  constexpr int32_t m2pi = int32_t(m2p*scale + 0.5);
  constexpr int32_t m2ti = int32_t(m2t*scale + 0.5);
  constexpr int32_t s2ii = int32_t(s2i*scale + 0.5);
  constexpr int32_t s2pi = int32_t(s2p*scale + 0.5);
  constexpr int32_t s2ti = int32_t(s2t*scale + 0.5);
  constexpr int32_t lim = 32767*scale;

  int32_t lp = LMS2IPT[src.L>32767 ? 32767 : src.L];
  int32_t mp = LMS2IPT[src.M>32767 ? 32767 : src.M];
  int32_t sp = LMS2IPT[src.S>32767 ? 32767 : src.S];

  int32_t i0 = l2ii*lp + m2ii*mp + s2ii*sp;
  int32_t p0 = l2pi*lp + m2pi*mp + s2pi*sp;
  int32_t t0 = l2ti*lp + m2ti*mp + s2ti*sp;

  dst.I = (i0<0) ? 0 : (i0>=lim) ? 65535 : (i0>>(logscale-1));
  dst.P = (p0<=-lim) ? -32767 : (p0>=lim) ? 32767 : (p0>>logscale);
  dst.T = (t0<=-lim) ? -32767 : (t0>=lim) ? 32767 : (t0>>logscale);
}

template <> inline void convert<IPT, LMS>(IPT const &src, LMS &dst) {
  constexpr int32_t logscale = 15;
  constexpr int32_t scale = 1<<logscale;
  constexpr int32_t iscale = 1<<(logscale-1);
  constexpr int32_t i2li = int32_t(i2l*iscale + 0.5);
  constexpr int32_t i2mi = int32_t(i2m*iscale + 0.5);
  constexpr int32_t i2si = int32_t(i2s*iscale + 0.5);
  constexpr int32_t p2li = int32_t(p2l*scale + 0.5);
  constexpr int32_t p2mi = int32_t(p2m*scale + 0.5);
  constexpr int32_t p2si = int32_t(p2s*scale + 0.5);
  constexpr int32_t t2li = int32_t(t2l*scale + 0.5);
  constexpr int32_t t2mi = int32_t(t2m*scale + 0.5);
  constexpr int32_t t2si = int32_t(t2s*scale + 0.5);
  constexpr int32_t lim = 32767*scale;

  int32_t i0 = src.I;
  int32_t p0 = src.P;
  int32_t t0 = src.T;
  int32_t lp = i2li*i0 + p2li*p0 + t2li*t0;
  int32_t mp = i2mi*i0 + p2mi*p0 + t2mi*t0;
  int32_t sp = i2si*i0 + p2si*p0 + t2si*t0;
  dst.L = IPT2LMS[lp<0 ? 0 : lp>=lim ? 32767 : (lp>>logscale)];
  dst.M = IPT2LMS[mp<0 ? 0 : mp>=lim ? 32767 : (mp>>logscale)];
  dst.S = IPT2LMS[sp<0 ? 0 : sp>=lim ? 32767 : (sp>>logscale)];
}

template <> inline void convert<sRGB, IPT>(sRGB const &src, IPT &dst) {
  LMS tmp;
  convert(src, tmp);
  convert(tmp, dst);
}

template <> inline void convert<XYZ, IPT>(XYZ const &src, IPT &dst) {
  LMS tmp;
  convert(src, tmp);
  convert(tmp, dst);
}

template <> inline void convert<IPT, sRGB>(IPT const &src, sRGB &dst) {
  LMS tmp;
  convert(src, tmp);
  convert(tmp, dst);
}
  
template <> inline void convert<IPT, XYZ>(IPT const &src, XYZ &dst) {
  LMS tmp;
  convert(src, tmp);
  convert(tmp, dst);
}

#endif
