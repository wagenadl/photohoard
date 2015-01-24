// CMSProfile.cpp

#include "CMSProfile.h"
#include <QAtomicInt>
#include <QApplication>
#include <QDesktopWidget>
#include <QX11Info>
#include <X11/Xutil.h>
#include <X11/Xlib.h>
#include <QMap>

void CMSProfile::initref() {
  refctr = new QAtomicInt;
  prof = NULL;
  ref();
}

bool CMSProfile::isValid() const {
  return prof != NULL;
}

CMSProfile CMSProfile::srgbProfile() {
  static CMSProfile p;
  if (!p.prof)
    p.prof = cmsCreate_sRGBProfile();
  return p;
}

CMSProfile CMSProfile::linearRgbProfile() {
  static CMSProfile p;
  if (!p.prof) {  
    cmsCIExyY D65;
    cmsWhitePointFromTemp(&D65, 6504);
    
    cmsCIExyYTRIPLE Rec709Primaries = {
      {0.6400, 0.3300, 1.0},
      {0.3000, 0.6000, 1.0},
      {0.1500, 0.0600, 1.0}
    };
    
    cmsToneCurve *curves[3];
    for (int c=0; c<3; c++)
      curves[c] = cmsBuildGamma(NULL, 1);

    p.prof = cmsCreateRGBProfile(&D65, &Rec709Primaries, curves);
  }
  return p;
}

CMSProfile CMSProfile::labProfile(double x_white,
                                  double y_white,
                                  double Y_white) {
  static CMSProfile p;
  if (!p.prof) {
    cmsCIExyY white;
    white.x = x_white;
    white.y = y_white;
    white.Y = Y_white;
    p.prof = cmsCreateLab2Profile(&white);
  }
  return p;
}

CMSProfile CMSProfile::xyzProfile() {
  static CMSProfile p;
  if (!p.prof) 
    p.prof = cmsCreateXYZProfile();
  
  return p;
}

CMSProfile CMSProfile::nullProfile() {
  static CMSProfile p;
  if (!p.prof)
    p.prof = cmsCreateNULLProfile();
  return p;
}

CMSProfile::CMSProfile() {
  initref();
}

CMSProfile::CMSProfile(QString filename) {
  initref();
  prof = cmsOpenProfileFromFile(filename.toUtf8().data(), "r");
}

void CMSProfile::load(QString filename) {
  *this = CMSProfile(filename);
}


CMSProfile::CMSProfile(CMSProfile const &o) {
  refctr = o.refctr;
  ref();
  prof = o.prof;
}

CMSProfile::~CMSProfile() {
  deref();
}

void CMSProfile::deref() {
  if (!refctr->deref()) {
    // deleted last copy
    delete refctr;
    if (isValid())
      cmsCloseProfile(prof);
  }
}

void CMSProfile::ref() {
  refctr->ref();
}

CMSProfile &CMSProfile::operator=(CMSProfile const &o) {
  if (o.refctr == refctr)
    return *this;
  deref();
  refctr = o.refctr;
  ref();
  prof = o.prof;
  return *this;
}

CMSProfile CMSProfile::displayProfile() {
  QDesktopWidget *desktop = QApplication::desktop();
  if (!desktop)
    return CMSProfile();
  QX11Info const &x11 = desktop->x11Info();
  Display *display = x11.display();
  int screen = x11.screen();
  Window root = RootWindow(display, screen);
  QString atomname = "_ICC_PROFILE";
  if (screen>0)
    atomname += QString("_%1").arg(screen);

  Atom iccAtom = XInternAtom(display, atomname.toUtf8().data(), true);
  Atom type;
  int format;
  unsigned long nItems, bytesAfter;
  unsigned char *data = 0;

  int result = XGetWindowProperty(display,
                                  root,
                                  iccAtom,
                                  0L,
                                  ~0L,
                                  False,
                                  AnyPropertyType,
                                  &type,
                                  &format,
                                  &nItems,
                                  &bytesAfter,
                                  &data);
  if (result!=Success || nItems==0)
    return CMSProfile();

  int length = 0;
  switch (format) {
  case 8:
    length = nItems;
    break;
  case 16:
    length = 2 * nItems;
    break;
  case 32:
    length = 4 * nItems;
    break;
  default:
    XFree(data);
    return CMSProfile();
  }
  
  CMSProfile p;
  p.prof = cmsOpenProfileFromMem(data, length);

  XFree(data);

  return p;
}

double CMSProfile::standardIlluminant_x(CMSProfile::StandardIlluminant il) {
  static QMap<StandardIlluminant, double> x;
  if (x.isEmpty()) {
    x[StandardIlluminant::A] = 0.44757;
    x[StandardIlluminant::B] = 0.34842;
    x[StandardIlluminant::C] = 0.31006;
    x[StandardIlluminant::D50] = 0.34567;
    x[StandardIlluminant::D55] = 0.33242;
    x[StandardIlluminant::D65] = 0.31271;
    x[StandardIlluminant::D75] = 0.29902;
    x[StandardIlluminant::E] = 1.0/3;

    x[StandardIlluminant::A_10] = 0.45117;
    x[StandardIlluminant::B_10] = 0.34980;
    x[StandardIlluminant::C_10] = 0.31039;
    x[StandardIlluminant::D50_10] = 0.34773;
    x[StandardIlluminant::D55_10] = 0.33411;
    x[StandardIlluminant::D65_10] = 0.31382;
    x[StandardIlluminant::D75_10] = 0.29968;
  }
  return x[il];
}

double CMSProfile::standardIlluminant_y(CMSProfile::StandardIlluminant il) {
  static QMap<StandardIlluminant, double> y;
  if (y.isEmpty()) {
    y[StandardIlluminant::A] = 0.40745;
    y[StandardIlluminant::B] = 0.35161;
    y[StandardIlluminant::C] = 0.31616;
    y[StandardIlluminant::D50] = 0.35850;
    y[StandardIlluminant::D55] = 0.34743;
    y[StandardIlluminant::D65] = 0.32902;
    y[StandardIlluminant::D75] = 0.31485;
    y[StandardIlluminant::E] = 1.0/3;

    y[StandardIlluminant::A_10] = 0.40594;
    y[StandardIlluminant::B_10] = 0.35270;
    y[StandardIlluminant::C_10] = 0.31905;
    y[StandardIlluminant::D50_10] = 0.35952;
    y[StandardIlluminant::D55_10] = 0.34877;
    y[StandardIlluminant::D65_10] = 0.33100;
    y[StandardIlluminant::D75_10] = 0.31740;
  }
  return y[il];
}
  
 
CMSProfile CMSProfile::labProfile(CMSProfile::StandardIlluminant il) {
  return labProfile(standardIlluminant_x(il),
                    standardIlluminant_y(il),
                    1);
  // Should Y=0.54 ?
}

CMSProfile::ColorSpace CMSProfile::colorSpace() const {
  switch (signature()) {
  case 0x52474220: return ColorSpace::RGB;
  case 0x58595a20: return ColorSpace::XYZ;
  case 0x4c616220: return ColorSpace::Lab;
  default: return ColorSpace::Unknown;
  }
}

int CMSProfile::signature() const {
  return prof ? cmsGetColorSpace(prof) : 0;
}
