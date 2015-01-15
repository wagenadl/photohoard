// CMSProfile.cpp

#include "CMSProfile.h"
#include <QAtomicInt>
#include <QApplication>
#include <QDesktopWidget>
#include <QX11Info>
#include <X11/Xutil.h>
#include <X11/Xlib.h>

void CMSProfile::initref() {
  refctr = new QAtomicInt;
  prof = NULL;
  ref();
}

bool CMSProfile::isValid() const {
  return prof != NULL;
}

CMSProfile CMSProfile::srgbProfile() {
  CMSProfile p;
  p.prof = cmsCreate_sRGBProfile();
  return p;
}

CMSProfile CMSProfile::labProfile(double x_white,
                                  double y_white,
                                  double Y_white) {
  CMSProfile p;
  cmsCIExyY white;
  white.x = x_white;
  white.y = y_white;
  white.Y = Y_white;
  p.prof = cmsCreateLab2Profile(&white);
  return p;
}

CMSProfile CMSProfile::xyzProfile() {
  CMSProfile p;
  p.prof = cmsCreateXYZProfile();
  return p;
}

CMSProfile CMSProfile::nullProfile() {
  CMSProfile p;
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
