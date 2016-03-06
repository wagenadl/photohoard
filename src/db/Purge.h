// Purge.h

#ifndef PURGE_H

#include <QSet>
#include <QMap>

#define PURGE_H

class Purge {
public:
  Purge(class PhotoDB *db, class AutoCache *cache);
  void refresh();
public:
  QSet<quint64> versionsWithSiblings() const { return vsnsWSiblings; }
  QSet<quint64> versionsWithoutSiblings() const { return vsnsWoSiblings; }
  QSet<quint64> photosWithOtherVersions() const { return phsWOtherVsns; }
  QSet<quint64> photosWithoutOtherVersions() const { return phsWoOtherVsns; }
  QMap<quint64, QString> photosThatCanBeDeleted() const {
    return phsThatCanBeDeld;
  } // map is photo ID to pathname
  QSet<quint64> orphanedPhotos() const { return orphans; }
  QMap<quint64, QString> orphansThatCanBeDeleted() const {
    return orphsThatCanBeDeld;
  } // map is photo ID to pathname
private:
  class PhotoDB *db;
  class AutoCache *cache;
  QSet<quint64> vsnsWSiblings;
  QSet<quint64> vsnsWoSiblings;
  QSet<quint64> phsWOtherVsns;
  QSet<quint64> phsWoOtherVsns;
  QMap<quint64, QString> phsThatCanBeDeld;
  QSet<quint64> orphans;
  QMap<quint64, QString> orphsThatCanBeDeld;
};

#endif
