// ImageConverters.cpp

#include "ImageConverters.h"

namespace ImageConverters {
  
  Image convertedToGray8(Image const &src) {
    switch (src.format()) {
    case Image::Format::Gray8:
      return src;
    case Image::Format::sRGB: {
      uint X = src.width();
      uint Y = src.height();
      uint dLs = src.bytesPerLine() - 4*X;
      Image dst(X, Y, Image::Format::Gray8);
      uint dLd = dst.bytesPerLine() - X;
      uint8 const *s = src.bits();
      uint8 *d = dst.bits();
      for (uint y=0; y<Y; y++) {
        for (uint x=0; x<X; x++) {
          quint16 p = *s++; // B
          p += quint16(*s++)*3; // G
          p += quint16(*s++)*2; // R
          s++; // A
          *d++ = p/6;
        }
        s += dLs;
        d += dLd;
      }
      return dst;
    }
    case Image::Format::Gray16: {
      uint X = src.width();
      uint Y = src.height();
      uint dLs = src.bytesPerLine() - 2*X;
      Image dst(X, Y, Image::Format::Gray8);
      uint dLd = dst.bytesPerLine() - X;
      uint8 const *s = src.bits();
      uint8 *d = dst.bits();
      s++;
      for (uint y=0; y<Y; y++) {
        for (uint x=0; x<X; x++) {
          *d++ = *s;
          s+= 2;
        }
        s += dLs;
        d += dLd;
      }
      return dst;
    }
    case Image::Format::RGB16: 
      return convertedToGray8(convertedToGray16(src));
    default: 
      return convertedToGray8(convertedToGray16(convertedToRGB16(src)));
    }
    return Image(); // not executed
  }
  
  Image convertedToGray16(Image const &src) {
    switch (src.format()) {
    case Image::Format::Gray8: {
      uint X = src.width();
      uint Y = src.height();
      uint dLs = src.bytesPerLine() - X;
      Image dst(X, Y, Image::Format::Gray16);
      uint dLd = dst.bytesPerLine() - 2*X;
      uint8 const *s = src.bits();
      uint8 *d = dst.bits();
      memset(d, 0, X*Y*2);
      d++;
      for (uint y=0; y<Y; y++) {
        for (uint x=0; x<X; x++) {
          *d = *s++;
          d += 2;
        }
        s += dLs;
        d += dLd;
      }
      return dst;
    }
    case Image::Format::sRGB8:
      return convertedToGray16(convertedToGray8(src));
    case Image::Format::Gray16:
      return src;
    case Image::Format::RGB16: {
      uint X = src.width();
      uint Y = src.height();
      uint dLs = (src.bytesPerLine() - 6*X)/2;
      Image dst(X, Y, Image::Format::Gray8);
      uint dLd = (dst.bytesPerLine() - 2*X)/2;
      uint16 const *s = (uint16 const *)(src.constBits());
      uint16 *d = (uint16 *)(dst.bits());
      for (uint y=0; y<Y; y++) {
        for (uint x=0; x<X; x++) {
          quint32 p = *s++; // B
          p += quint16(*s++)*3; // G
          p += quint16(*s++)*2; // R
          *d++ = p/6;
        }
        s += dLs;
        d += dLd;
      }
      return dst;
    }
    default:
      return convertedToGray16(convertedToRGB16(src));
    }
    return Image(); // not executed
  }
      
  Image convertedTosRGB8(Image const &src) {
    switch (src.format() {
      case Image::Format::Gray8: {
        uint X = src.width();
        uint Y = src.height();
        uint dLs = src.bytesPerLine() - X;
        Image dst(X, Y, Image::Format::sRGB8);
        uint dLd = dst.bytesPerLine() - 4*X;
        uint8 const *s = src.bits();
        uint8 *d = dst.bits();
        for (uint y=0; y<Y; y++) {
          for (uint x=0; x<X; x++) {
            quint8 p = *s++;
            *d++ = p; // B
            *d++ = p; // G
            *d++ = p; // R
            *d++ = 255; // A
          }
          s += dLs;
          d += dLd;
        }
        return dst;
      }
      case Image::Format::sRGB8:
        return src;
      case Image::Format::Gray16: {
        uint X = src.width();
        uint Y = src.height();
        uint dLs = src.bytesPerLine() - 2*X;
        Image dst(X, Y, Image::Format::sRGB8);
        uint dLd = dst.bytesPerLine() - 4*X;
        uint8 const *s = src.bits();
        uint8 *d = dst.bits();
        s++; 
        for (uint y=0; y<Y; y++) {
          for (uint x=0; x<X; x++) {
            quint8 p = *s;
            s += 2;
            *d++ = p; // B
            *d++ = p; // G
            *d++ = p; // R
            *d++ = 255; // A
          }
          s += dLs;
          d += dLd;
        }
        return dst;
      }
      case Image::Format::RGB16: {
        uint X = src.width();
        uint Y = src.height();
        uint dLs = src.bytesPerLine() - 6*X;
        Image dst(X, Y, Image::Format::sRGB8);
        uint dLd = dst.bytesPerLine() - 4*X;
        uint8 const *s = src.bits();
        uint8 *d = dst.bits();
        s++; 
        for (uint y=0; y<Y; y++) {
          for (uint x=0; x<X; x++) {
            *d++ = *s; // B
            s+=2;
            *d++ = *s; // G
            s+=2;
            *d++ = *s; // R
            s+=2;
            *d++ = 255; // A
          }
          s += dLs;
          d += dLd;
        }
        return dst;
      }
      }
  Image convertedToRGB16(Image const &src);
  Image convertedToXYZ16(Image const &src);
  Image convertedToLab16(Image const &src);
  Image convertedToRGB14(Image const &src);
  Image convertedToXYZ14(Image const &src);
  Image convertedToLab14(Image const &src);
  bool convertToRGB16(Image &src);
  bool convertToXYZ16(Image &src);
  bool convertToLab16(Image &src);
  bool convertToRGB14(Image &src);
  bool convertToXYZ14(Image &src);
  bool convertToLab14(Image &src);
