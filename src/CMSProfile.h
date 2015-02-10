// CMSProfile.h

#ifndef CMSPROFILE_H

#define CMSPROFILE_H

#include <lcms2.h>
#include <QString>

class CMSProfile {
public:
  enum class StandardIlluminant {
    A, B, C, D50, D55, D65, D75, E,
    A_10, B_10, C_10, D50_10, D55_10, D65_10, D75_10,
      };
  enum class ColorSpace {
    Unknown, RGB, XYZ, Lab
  };
public:
  CMSProfile(QString filename);
  CMSProfile();
  void load(QString filename);
  CMSProfile(CMSProfile const &);
  ~CMSProfile();
  bool isValid() const;
  CMSProfile &operator=(CMSProfile const &);
  ColorSpace colorSpace() const;
public: // but rather technical
  cmsHPROFILE const &profile() const { return prof; }
  int signature() const;
public:
  static CMSProfile srgbProfile();
  static CMSProfile linearRgbProfile();
  static CMSProfile curvedRgbProfile(class CMSToneCurve const &r,
                                     CMSToneCurve const &g,
                                     CMSToneCurve const &b);
  static CMSProfile labProfile(double x_white, double y_white, double Y_white);
  static CMSProfile labProfile(StandardIlluminant il=StandardIlluminant::D50);
  static CMSProfile curvedLabProfile(class CMSToneCurve const &L,
                                     CMSToneCurve const &a,
                                     CMSToneCurve const &b); // uses D65
  static CMSProfile xyzProfile();
  static CMSProfile curvedXYZProfile(class CMSToneCurve const &x,
                                     CMSToneCurve const &y,
                                     CMSToneCurve const &z);
  static CMSProfile nullProfile();
  static CMSProfile displayProfile();
private:
  void initref();
  void ref();
  void deref();
  static double standardIlluminant_x(StandardIlluminant il);
  static double standardIlluminant_y(StandardIlluminant il);
private:
  class QAtomicInt *refctr;
  cmsHPROFILE prof;
};

#endif
