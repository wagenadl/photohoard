// Image16Base.h

#ifndef IMAGE16BASE_H

#define IMAGE16BASE_H

class Image16Base {
public:
  enum class Format {
    sRGB8,
    XYZ16,
    XYZp16,
    Lab16,
    LMS16,
    IPT16
  };
  enum class Interpolation {
    NearestNeighbor,
    BoxcarAverage,
    Linear,
    Area,
    Cubic,
    Lanczos
  };
  enum class Crop { // For rotate and perspective transforms
    Same, // i.e., same area as source image
    MaxDefinedArea, // i.e., maximum area that has no undefined pixels
    MinInclusiveArea // i.e., minimum area that excludes no defined pixels
  };
};

#endif
