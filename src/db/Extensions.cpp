// Extensions.cpp

#include "Extensions.h"

namespace Extensions {
  QSet<QString> const &imageExtensions() {
    static QSet<QString> exts;
    if (exts.isEmpty()) 
      exts << "jpg" << "png" << "jpeg" << "tif" << "tiff" << "cr2" << "nef";
    // Keep this in sync with setupdb.sql!
    return exts;
  }
  QSet<QString> const &movieExtensions() {
    static QSet<QString> exts;
    if (exts.isEmpty())
      exts << "mp4" << "mov";
    return exts;
  }
}
