// MetaInfo.h

#ifndef METAINFO_H

#define METAINFO_H

#include <QString>
#include "PSize.h"

class MetaInfo {
public:
  MetaInfo(class PhotoDB *db, quint64 version);
  QString html() const { return txt; }
public:
  static QString ratio(int w, int h);
  static QString ratio(PSize);
  static QString easyRatio(int w, int h);
  static QString easyRatio(PSize);
  static QString mpix(int w, int h);
  static QString mpix(PSize);
private:
  QString txt;
};

#endif
