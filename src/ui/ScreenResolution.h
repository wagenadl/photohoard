// ScreenResolution.h

#ifndef SCREENRESOLUTION_H

#define SCREENRESOLUTION_H

#include <QSizeF>

class ScreenResolution {
public:
  ScreenResolution(class QApplication *);
  QSize pixelCount() const;
  QSizeF millimeterSize() const;
  int dpi() const;
private:
  static QSize &pc();
  static QSizeF &ms();
  static int &dpi_();
  static bool &ready();
private:
  static void ensure();
  static void fallback();
};

#endif
