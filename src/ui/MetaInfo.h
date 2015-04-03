// MetaInfo.h

#ifndef METAINFO_H

#define METAINFO_H

#include <QString>

class MetaInfo {
public:
  MetaInfo(class PhotoDB const &db, quint64 version);
  QString html() const { return txt; }
public:
  static QString ratio(int w, int h);
  static QString mpix(int w, int h);
private:
  QString txt;
};

#endif
