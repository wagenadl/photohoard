// CS_Lab.h

#ifndef CS_LAB_H

#define CS_LAB_H

constexpr uint32_t lab_tablesize = 32768;
extern uint16_t LabFwd[lab_tablesize];
extern uint16_t LabRev[lab_tablesize];

template <> inline void convert<XYZ, XYZp>(XYZ const &src, XYZp &dst) {
  constexpr int32_t logscale = 16;
  constexpr int32_t scale = 1<<logscale;
  constexpr int32_t xscale = int32_t(scale/d65_X + 0.5);
  constexpr int32_t zscale = int32_t(scale/d65_Z + 0.5);

  uint32_t x0 = src.X;
  uint32_t z0 = src.Z;

  x0 *= xscale;
  z0 *= zscale;

  dst.Xp = x0>>logscale;
  dst.Y = src.Y;
  dst.Zp = z0>>logscale;
}

template <> inline void convert<XYZp, Lab>(XYZp const &src, Lab &dst) {
  constexpr uint16_t mask = 0x8000;
  uint32_t x0 = LabFwd[(src.Xp&mask) ? lab_tablesize-1 : src.Xp];
  uint32_t y0 = LabFwd[(src.Y&mask) ? lab_tablesize-1 : src.Y];
  uint32_t z0 = LabFwd[(src.Zp&mask) ? lab_tablesize-1 : src.Zp];

  constexpr int32_t Lscale = 76024; // approx 116×655.35
  constexpr int32_t Loffset = 10486; // approx 16×655.35
  
  dst.L = Lscale*y0/32768 - Loffset;
  dst.a = (500*(x0-y0)+64)/128;
  dst.b = (200*(y0-z0)+64)/128;
}

template <> inline void convert<XYZ, Lab>(XYZ const &src, Lab &dst) {
  XYZp tmp;
  convert(src, tmp);
  convert(tmp, dst);
}

template <> inline void convert<Lab, XYZp>(Lab const &src, XYZp &dst) {
  int32_t L = src.L;
  int32_t a = src.a;
  int32_t b = src.b;
  constexpr uint32_t Lscale = 76021; // approx 655.35×116
  constexpr uint32_t Loffset = 4519; // approx ?
  uint16_t L1 = 32768*L/Lscale + Loffset; //  L' = 32767 [(1/116) (L*+16)]
  uint16_t a1 = L1 + 32*a/125; //  a' = 32767 [(1/116) (L*+16) + (1/500) a*]
  uint16_t b1 = L1 - 80*b/125; //  a' = 32767 [(1/116) (L*+16) - (1/200) b*]
  dst.Xp = LabRev[a1];
  dst.Y = LabRev[L1];
  dst.Zp = LabRev[b1];
}

template <> inline void convert<XYZp, XYZ>(XYZp const &src, XYZ &dst) {
  constexpr int32_t xscale = int32_t(d65_X * 65536 + 0.5);
  constexpr int32_t zscale = int32_t(d65_Z * 65536 + 0.5);
  uint32_t X0 = src.Xp;
  uint32_t Z0 = src.Zp;
  dst.X = (xscale*X0) / 65536;
  dst.Y = src.Y;
  dst.Z = (zscale*Z0) / 65536;
}    

template <> inline void convert<Lab, XYZ>(Lab const &src, XYZ &dst) {
  XYZp tmp;
  convert(src, tmp);
  convert(tmp, dst);
}

#endif
