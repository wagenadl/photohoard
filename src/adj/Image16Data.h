// Image16Data.h

#ifndef IMAGE16DATA_H

#define IMAGE16DATA_H

#include "Image16Base.h"
#include <QSharedPointer>

class Image16Data: public QSharedData {
public:
  Image16Data(int w=0, int h=0,
	      Image16Base::Format f=Image16Base::Format::sRGB8):
    width(w), height(h), format(f),
    image(format==Image16Base::Format::sRGB8 ? w : 3*w, h,
	  format==Image16Base::Format::sRGB8 ? QImage::Format_RGB32
	  : QImage::Format_RGB16), roibyteoffset(0) {
    bytesperline = image.bytesPerLine();
  }
  Image16Data(QImage const &img):
    width(img.width()), height(img.height()),
    format(Image16Base::Format::sRGB8),
    image(img.convertToFormat(QImage::Format_RGB32)),
    roibyteoffset(0) {
    bytesperline = image.bytesPerLine();
  }
public:
  int width;
  int height;
  int bytesperline;
  Image16Base::Format format;
  QImage image;
  int roibyteoffset;
};


#endif
