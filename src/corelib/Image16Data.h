// Image16Data.h

#ifndef IMAGE16DATA_H

#define IMAGE16DATA_H

#include "Image16Base.h"
#include <QSharedPointer>

class Image16Data: public QSharedData {
public:
  Image16Data(int w=0, int h=0,
	      Image16Base::Format f=Image16Base::Format::sRGB8);
  Image16Data(QImage const &img,
              Image16Base::Format f=Image16Base::Format::sRGB8);
  /* THIS DOES NOT WORK
  Image16Data(Image16Data *src, QRect subimg);
  */
  virtual ~Image16Data();
  inline int bytesPerPixel() const { return is8Bits() ? 4 : 6; }
  inline bool is8Bits() const { return format == Image16Base::Format::sRGB8; }
public:
  int width;
  int height;
  int bytesperline;
  Image16Base::Format format;
  QImage image;
  int roibyteoffset;
};


#endif
