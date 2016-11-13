// ScreenResolution.h

#ifndef SCREENRESOLUTION_H

#define SCREENRESOLUTION_H

#include <QSizeF>

class ScreenResolution {
public:
  ScreenResolution();
  QSize pixelCount() const;
  QSizeF millimeterSize() const;
  double dpi() const;
private:
  static QSize &pc();
  static QSizeF &ms();
  static double &dpi_();
  static bool &ready();
private:
  static void ensure();
  static void fallback();
};

#endif
