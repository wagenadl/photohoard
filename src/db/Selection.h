// Selection.h

#ifndef SELECTION_H

#define SELECTION_H

#include "PhotoDB.h"
#include <QDateTime>
#include "Strip.h"
#include <QSet>

class Selection {
public:
  Selection(PhotoDB *photodb);
  int count();
  QSet<quint64> current();
  void add(quint64 vsn);
  void addDateRange(QDateTime d1, QDateTime inclusiveend);
  void addDateRange(QDateTime start, Strip::TimeScale scl);
  void dropDateRange(QDateTime start, QDateTime inclusiveend);
  void dropDateRange(QDateTime start, Strip::TimeScale scl);
  void selectAll();
  void addInFolder(QString folder);
  void addInTree(QString folder);
  void dropInFolder(QString folder);
  void dropInTree(QString folder);
  void addRestOfFolder(quint64 folder, QDateTime startAt);
  void addStartOfFolder(quint64 folder, QDateTime endAt);
  void addFoldersBetween(quint64 fid1, quint64 fid2); // not including fid1 and fid2!
  // current impl. _does_ know about filters for all of the above
  void remove(quint64 vsn);
  void clear();
  bool contains(quint64 vsn);
  int countInDateRange(QDateTime t0, QDateTime t1) const; // ... the
  int countInFolder(QString folder) const; // ... corresponding functions in
  int countInTree(QString folder) const; // ... PhotoDB except restricted to
  // ... selection
private:
  PhotoDB *db;
};

#endif
