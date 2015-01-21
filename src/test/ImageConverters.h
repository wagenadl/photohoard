// ImageConverters.h

#ifndef IMAGECONVERTERS_H

#define IMAGECONVERTERS_H

namespace ImageConverters {
  /* In all cases, s represents the color space of the source.
     The output should probably be in the same color space where possible.
  */
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
