// ScreenResolution.h

#ifndef SCREENRESOLUTION_H

#define SCREENRESOLUTION_H

#include <QSizeF>
#include <QFont>

class ScreenResolution {
public:
  static QSize pixelCount();
  static QSizeF millimeterSize();
  static double dpi();
  static QFont defaultFont();
private:
  static QSize &pc();
  static QSizeF &ms();
  static double &dpi_();
  static bool &ready();
private:
  static void ensure();
};

#endif
