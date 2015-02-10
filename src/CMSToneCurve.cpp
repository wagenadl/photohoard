// CMSToneCurve.cpp

#include "CMSToneCurve.h"

namespace CMS {
  Segment::Segment(Gamma gamma) {
    params << gamma.gamma;
    if (gamma.post.scale==1 && gamma.post.offset==0) {
      if (gamma.pre.scale==1 && gamma.pre.offset==0) {
        code = 1;
      } else {
        code = 2;
        params << gamma.pre.scale;
        params << gamma.post.scale;
      }
    } else {
      code = 3;
      double fac = pow(gamma.post.scale, 1/gamma.gamma);
      params << gamma.pre.scale * fac;
      params << gamma.pre.offset * fac;
      params << gamma.post.offset;
    }
  }
  
  Segment::Segment(Gamma gamma, Constant c, Threshold thr) {
    bool autothr = thr.thr < 0;
    if (autothr)
      thr.thr = -gamma.pre.offset/gamma.pre.scale;
    if (c.c < 0) 
      c.c = pow(gamma.pre.scale*thr.thr+gamma.pre.offset, gamma.gamma)
        * gamma.post.scale + gamma.post.offset;
    params << gamma.gamma;
    if (autothr && c.c==gamma.post.offset) {
      code = 3;
      double fac = pow(gamma.post.scale, 1/gamma.gamma);
      params << gamma.pre.scale * fac;
      params << gamma.pre.offset * fac;
      params << gamma.post.offset;
    } else {
      code = 5;
      double fac = pow(gamma.post.scale, 1/gamma.gamma);
      params << gamma.pre.scale * fac;
      params << gamma.pre.offset * fac;
      params << 0;
      params << thr.thr;
      params << gamma.post.offset;
      params << c.c;
    }
  }

  Segment::Segment(Gamma gamma, Linear lin, Threshold thr) {
    if (thr.thr<0)
      thr.thr = -gamma.pre.offset / gamma.pre.scale;
    params << gamma.gamma;
    code = 5;
    double fac = pow(gamma.post.scale, 1/gamma.gamma);
    params << gamma.pre.scale * fac;
    params << gamma.pre.offset * fac;
    params << lin.scale;
    params << thr.thr;
    params << gamma.post.offset;
    params << lin.offset;
  }
    
  Segment::Segment(Gamma gamma, Threshold thr) {
    if (thr.thr<0)
      thr.thr = -gamma.pre.offset / gamma.pre.scale;
    params << gamma.gamma;
    if (gamma.post.scale==1 && gamma.post.offset==0) {
      code = 4;
      params << gamma.pre.scale;
      params << gamma.pre.offset;
      params << pow(gamma.pre.scale*thr.thr+gamma.pre.offset,gamma.gamma)
        / thr.thr;
      params << thr.thr;
    } else {
      code = 5;
      double fac = pow(gamma.post.scale, 1/gamma.gamma);
      params << gamma.pre.scale * fac;
      params << gamma.pre.offset * fac;
      params << (pow(gamma.pre.scale*thr.thr+gamma.pre.offset,gamma.gamma)
                 * gamma.post.scale + gamma.post.offset) / thr.thr;
      params << thr.thr;
      params << gamma.post.offset;
      params << 0;
    }
  }
  
  Segment::Segment(Logarithmic logarithmic) {
    code = 7;
    params << logarithmic.gamma;
    params << logarithmic.post.scale;
    params << logarithmic.pre.scale;
    params << logarithmic.pre.offset;
    params << logarithmic.post.offset;
  }
    
  Segment::Segment(Power power) {
    code = 8;
    params << 1; // really? Yes, according to LittleCMS2.6 API p. 133
    params << power.post.scale;
    params << power.base;
    params << power.pre.scale;
    params << power.pre.offset;
    params << power.post.offset;
  }
    
  Segment::Segment(Sigmoidal sigmoidal) {
    code = 108;
    params << sigmoidal.gamma;
  }
    
  Segment::Segment(QVector<float> const &samples):
    code(0), samples(samples) {
  }

  Segment::Segment(int code, QVector<double> const &params):
    code(code), params(params) {
  }

};

CMSToneCurve::CMSToneCurve() {
  ctr = 0;
  d = 0;
}

CMSToneCurve::CMSToneCurve(CMSToneCurve const &o) {
  ctr = o.ctr;
  d = o.d;
  ref();
}

CMSToneCurve &CMSToneCurve::operator=(CMSToneCurve const &o) {
  if (d == o.d) 
    return *this;
  deref();
  ctr = o.ctr;
  d = o.d;
  ref();
  return *this;
}

CMSToneCurve::~CMSToneCurve() {
  deref();
}

CMSToneCurve::CMSToneCurve(CMS::Segment const &seg) {
  ctr = new int(1);
  if (seg.code==0) 
    d = cmsBuildTabulatedToneCurveFloat(NULL,
                                        seg.samples.size(),
                                        seg.samples.data());
  else
    d = cmsBuildParametricToneCurve(NULL,
                                    seg.code,
                                    seg.params.data());
}

static void makeCmsCurveSegment(cmsCurveSegment *out, CMS::Segment const &in) {
  out->Type = in.code;
  if (in.code) {
    int K = in.params.size();
    for (int k=0; k<K; k++)
      out->Params[k] = in.params[k];
    for (int k=K; k<10; k++)
      out->Params[k] = 0;
  } else {
    int N = in.samples.size();
    out->nGridPoints = N;
    out->SampledPoints = const_cast<float*>(in.samples.data());
  }
}
    
CMSToneCurve::CMSToneCurve(CMS::Segment const &s1,
                           double thr, CMS::Segment const &s2) {
  ctr = new int(1);
  cmsCurveSegment segs[2];
  segs[0].x0 = 0;
  segs[0].x1 = thr;
  makeCmsCurveSegment(&segs[0], s1);
  segs[1].x0 = thr;
  segs[1].x1 = 1;
  makeCmsCurveSegment(&segs[1], s2);
  d = cmsBuildSegmentedToneCurve(NULL, 2, segs);
}

CMSToneCurve::CMSToneCurve(CMS::Segment const &s1,
                           double thr1, CMS::Segment const &s2,
                           double thr2, CMS::Segment const &s3) {
  ctr = new int(1);
  cmsCurveSegment segs[3];
  segs[0].x0 = 0;
  segs[0].x1 = thr1;
  makeCmsCurveSegment(&segs[0], s1);
  segs[1].x0 = thr1;
  segs[1].x1 = thr2;
  makeCmsCurveSegment(&segs[1], s2);
  segs[2].x0 = thr2;
  segs[2].x1 = 1;
  makeCmsCurveSegment(&segs[2], s3);
  d = cmsBuildSegmentedToneCurve(NULL, 3, segs);
}

CMSToneCurve::CMSToneCurve(CMS::Segment const &s1,
                           double thr1, CMS::Segment const &s2,
                           double thr2, CMS::Segment const &s3,
                           double thr3, CMS::Segment const &s4) {
  ctr = new int(1);
  cmsCurveSegment segs[4];
  segs[0].x0 = 0;
  segs[0].x1 = thr1;
  makeCmsCurveSegment(&segs[0], s1);
  segs[1].x0 = thr1;
  segs[1].x1 = thr2;
  makeCmsCurveSegment(&segs[1], s2);
  segs[1].x0 = thr2;
  segs[1].x1 = thr3;
  makeCmsCurveSegment(&segs[2], s3);
  segs[2].x0 = thr3;
  segs[2].x1 = 1;
  makeCmsCurveSegment(&segs[3], s4);
  d = cmsBuildSegmentedToneCurve(NULL, 4, segs);
}  
  
CMSToneCurve CMSToneCurve::inverse(int npoints) const {
  if (!isValid())
    return CMSToneCurve();
  CMSToneCurve res;
  res.ctr = new int(1);
  res.d = npoints ? cmsReverseToneCurve(d)
    : cmsReverseToneCurveEx(npoints, d);
  return res;
}
  
// Create a tone curve if the form Z = Y⁻¹[X]
CMSToneCurve CMSToneCurve::compose(CMSToneCurve const &X,
                                   CMSToneCurve const &Y, int npoints) {
  if (!X.isValid() || !Y.isValid())
    return CMSToneCurve();
  CMSToneCurve res;
  res.ctr = new int(1);
  res.d = cmsJoinToneCurve(NULL, X.d, Y.d, npoints);
  return res;
}

CMSToneCurve &CMSToneCurve::linear() {
  static CMSToneCurve res;
  if (!res.d) {
    res.ctr = new int(1);
    res.d = cmsBuildGamma(NULL, 1);
  }
  return res;
}
  
bool CMSToneCurve::isMultisegment() const {
  return isValid() ? cmsIsToneCurveMultisegment(d) : false;
}

bool CMSToneCurve::isRoughlyLinear() const {
  return isValid() ? cmsIsToneCurveLinear(d) : true;
}
  
bool CMSToneCurve::isRoughlyMonotonic() const {
  return isValid() ? cmsIsToneCurveMonotonic(d) : true;
}
  
bool CMSToneCurve::isDescending() const {
  return isValid() ? cmsIsToneCurveDescending(d) : false;
}

void CMSToneCurve::ref() {
  if (ctr)
    (*ctr)++;
}

void CMSToneCurve::deref() {
  if (ctr) {
    (*ctr)--;
    if (!*ctr) {
      if (d)
        cmsFreeToneCurve(d);
      delete ctr;
    }
  }
}
        
