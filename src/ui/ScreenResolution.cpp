// ScreenResolution.cpp

#include "ScreenResolution.h"

#include <QApplication>
#include <QDebug>
#include <QScreen>
#include <math.h>
#include <QMutexLocker>

QSize ScreenResolution::pixelCount() {
  ensure();
  qDebug() << "pixelcount" << pc();
  return pc();
}

QSizeF ScreenResolution::millimeterSize() {
  ensure();
  qDebug() << "millimetersize" << ms();
  return ms();
}

double ScreenResolution::dpi() {
  ensure();
  qDebug() << "dpi" << dpi_();
  return dpi_();
}

QFont ScreenResolution::defaultFont() {
  static QFont f("Lato");
  static bool first = true;
  if (first) 
    f.setPixelSize(0.1 * dpi() * pow(millimeterSize().width()/250, 0.3));
  return f;
}

//////////////////////////////////////////////////////////////////////
// Internal functions
QSize &ScreenResolution::pc() {
  static QSize p;
  return p;
}

QSizeF &ScreenResolution::ms() {
  static QSizeF m;
  return m;
}

double &ScreenResolution::dpi_() {
  static double d;
  return d;
}

bool &ScreenResolution::ready() {
  static bool ok = false;
  return ok;
}

void ScreenResolution::ensure() {
  // This mutex guarantees that once we are calculating, we won't start again.
  static QMutex mutex;
  QMutexLocker l(&mutex);
  
  if (ready())
    return;

  QGuiApplication *app
    = reinterpret_cast<QGuiApplication*>(QApplication::instance());
  if (!app) {
    qDebug() << "no gui app";
    return;
  }
  QScreen *scr = app->primaryScreen();

  ms() = scr->physicalSize();
  pc() = scr->size();
  double xdpi = pc().width()/(ms().width()/25.4);
  double ydpi = pc().height()/(ms().height()/25.4);
  dpi_() = sqrt(xdpi*ydpi);
  ready() = true;
}
