// CMSProfile.h

#ifndef CMSPROFILE_H

#define CMSPROFILE_H

#include <lcms2.h>
#include <QString>

class CMSProfile {
public:
  CMSProfile(QString filename);
  CMSProfile();
  void load(QString filename);
  CMSProfile(CMSProfile const &);
  ~CMSProfile();
  bool isValid() const;
  CMSProfile &operator=(CMSProfile const &);
  cmsHPROFILE const &profile() const { return prof; }
  static CMSProfile srgbProfile();
  static CMSProfile labProfile(double x_white, double y_white, double Y_white);
  static CMSProfile xyzProfile();
  static CMSProfile nullProfile();
  static CMSProfile displayProfile();
private:
  void initref();
  void ref();
  void deref();
private:
  class QAtomicInt *refctr;
  cmsHPROFILE prof;
};

#endif
