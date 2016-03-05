// Purge.h

#ifndef PURGE_H

#include <QSet>

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
  QSet<quint64> photosThatCanBeDeleted() const { return phsThatCanBeDeld; }
  QSet<quint64> photosThatCannotBeDeleted() const { return phsThatCantBeDeld; }
private:
  class PhotoDB *db;
  class AutoCache *cache;
  QSet<quint64> vsnsWSiblings;
  QSet<quint64> vsnsWoSiblings;
  QSet<quint64> phsWOtherVsns;
  QSet<quint64> phsWoOtherVsns;
  QSet<quint64> phsThatCanBeDeld;
  QSet<quint64> phsThatCantBeDeld;
};

#endif
