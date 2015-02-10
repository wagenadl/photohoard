// CMSToneCurve.h

#ifndef CMSTONECURVE_H

#define CMSTONECURVE_H

#include <lcms2.h>
#include <math.h>
#include <QList>
#include <QVector>

namespace CMS {
  class Constant {
  public:
    double c;
  public:
    explicit Constant(double c=0): c(c) { }
  };
  
  class Linear {
  public:
    double scale, offset;
  public:
    // scale X + offset
    explicit Linear(double scale=1, double offset=0): 
      scale(scale), offset(offset) { }
  };

  class Gamma {
  public:
    Linear pre;
    double gamma;
    Linear post;
  public:
    // X^gamma
    explicit Gamma(double gamma=1): 
      gamma(gamma) { }
    // a X^gamma + b
    explicit Gamma(double gamma, Linear post): 
      gamma(gamma), post(post) { }
    // aₚₒₛₜ (aₚᵣₑ X + bₚᵣₑ)^gamma + bₚₒₛₜ
    explicit Gamma(Linear pre, double gamma=1, Linear post=Linear()):
      pre(pre), gamma(gamma), post(post) { }
  };

  class Threshold {
  public:
    double thr;
  public:
    explicit Threshold(double thr=-1): thr(thr) { }
  };

  class Logarithmic {
  public:
    Linear pre;
    double gamma;
    Linear post;
  public:
    // aₚₒₛₜ log(aₚᵣₑ X^gamma + bₚᵣₑ) + bₚₒₛₜ
    explicit Logarithmic(Linear pre, double gamma=1, Linear post=Linear()):
      pre(pre), gamma(gamma), post(post) { }
  };

  class Power {
  public:
    Linear pre;
    double base;
    Linear post;
  public:
    // aₚₒₛₜ base^X + bₚₒₛₜ
    explicit Power(double base=exp(1), Linear post=Linear());
    // aₚₒₛₜ base^(aₚᵣₑ X + bₚᵣₑ) + bₚₒₛₜ
    explicit Power(Linear pre, double base=exp(1), Linear post=Linear());
  };

  class Sigmoidal {
  public:
    double gamma;
  public:
    // (1 - (1-X)^(1/gamma))^(1/gamma)
    explicit Sigmoidal(double gamma): gamma(gamma) { }
  };
    
  class Segment {
  public:
    // Simple gamma curve
    Segment(Gamma gamma);
    // Gamma curve if X>=thr; constant otherwise.
    // If thr is not given, it is set to -bₚᵣₑ/aₚᵣₑ.
    // If c<0, it is set to gamma(X=thr).
    Segment(Gamma gamma, Constant c, Threshold thr=Threshold());
    // Gamma curve if X>=thr; otherwise linear.
    Segment(Gamma gamma, Linear lin, Threshold thr=Threshold());
    // Gamma curve if X>=thr; otherwise a X, where a is such that
    // a thr = gamma(X=thr)
    Segment(Gamma gamma, Threshold thr);
    Segment(Logarithmic logarithmic);
    Segment(Power power);
    Segment(Sigmoidal sigmoidal);
    Segment(QVector<float> const &sampledPoints);
    Segment(int code, QVector<double> const &params);
  public:
    int code; // 0 for sampled
    QVector<double> params;
    QVector<float> samples;
  };
};

class CMSToneCurve {
public:
  CMSToneCurve();
  CMSToneCurve(CMSToneCurve const &);
  CMSToneCurve &operator=(CMSToneCurve const &);
  ~CMSToneCurve();
  CMSToneCurve(CMS::Segment const &);
  CMSToneCurve(CMS::Segment const &s1, double thr, CMS::Segment const &s2);
  CMSToneCurve(CMS::Segment const &s1, double thr1, CMS::Segment const &s2,
               double thr2, CMS::Segment const &s3);
  CMSToneCurve(CMS::Segment const &s1, double thr1, CMS::Segment const &s2,
               double thr2, CMS::Segment const &s3,
               double thr3, CMS::Segment const &s4);
  CMSToneCurve inverse(int npoints) const;
  // Create a tone curve if the form Z = Y⁻¹[X]
  static CMSToneCurve compose(CMSToneCurve const &X,
                              CMSToneCurve const &Y, int npoints);
  bool isMultisegment() const;
  bool isRoughlyLinear() const;
  bool isRoughlyMonotonic() const;
  bool isDescending() const;
  bool isValid() const { return d!=NULL; }
  cmsToneCurve *curve() const { return d; }
  static CMSToneCurve &linear();
private:
  void ref();
  void deref();
private:
  mutable int *ctr;
  cmsToneCurve *d;
};

#endif
