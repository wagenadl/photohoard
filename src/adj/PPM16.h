// PPM16.h

#ifndef PPMINFO_H

#define PPMINFO_H

#include <QByteArray>
#include <QImage>

class PPM16 {
public:
  PPM16(QByteArray const &);
  bool ok() const { return w>0; }
  int width() const { return w; }
  int height() const { return h; }
  QImage const &data() const { return dat; } // normalized 16 bits data
private:
  QImage dat;
  int w, h;  
};

#endif
