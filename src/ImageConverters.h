// ImageConverters.h

#ifndef IMAGECONVERTERS_H

#define IMAGECONVERTERS_H

namespace ImageConverters {
  Image gray8ToColor8(QImage const &img, Image::Space s);
  Image gray8ToGray16(QImage const &img, Image::Space s);
  Image gray8ToColor16(QImage const &img, Image::Space s);
  
  Image color8ToGray8(QImage const &img, Image::Space s);
  Image color8ToGray16(QImage const &img, Image::Space s);
  Image color8ToColor16(QImage const &img, Image::Space s);

  Image gray16ToGray8(QImage const &img, Image::Space s);
  Image gray16ToColor8(QImage const &img, Image::Space s);
  Image gray16ToColor16(QImage const &img, Image::Space s);
  
  Image color16ToGray8(QImage const &img, Image::Space s);
  Image color16ToColor8(QImage const &img, Image::Space s);
  Image color16ToGray16(QImage const &img, Image::Space s);
};

#endif
