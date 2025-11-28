// Selection.h

#ifndef SELECTION_H

#define SELECTION_H

#include "PhotoDB.h"
#include "DateTime.h"
#include "Strip.h"
#include <QSet>

class Selection {
public:
  Selection(PhotoDB *photodb);
  int count();
  QSet<quint64> current();
  void add(quint64 vsn);
  void addDateRange(DateTime const &d1, DateTime const &inclusiveend);
  void addDateRange(DateTime const &start, Strip::TimeScale scl);
  void dropDateRange(DateTime const &start, DateTime const &inclusiveend);
  void dropDateRange(DateTime const &start, Strip::TimeScale scl);
  void selectAll();
  void addInFolder(QString folder);
  void addInTree(QString folder);
  void dropInFolder(QString folder);
  void dropInTree(QString folder);
  void addRestOfFolder(quint64 folder, DateTime const &startAt);
  void addStartOfFolder(quint64 folder, DateTime const &endAt);
  void addFoldersBetween(quint64 fid1, quint64 fid2); // not including fid1 and fid2!
  // current impl. _does_ know about filters for all of the above
  void remove(quint64 vsn);
  void clear();
  bool contains(quint64 vsn);
  int countInDateRange(DateTime const &t0,
                       DateTime const &t1) const; // ... the
  int countInFolder(QString folder) const; // ... corresponding functions in
  int countInTree(QString folder) const; // ... PhotoDB except restricted to
  // ... selection
private:
  PhotoDB *db;
};

#endif
