// Selection.h

#ifndef SELECTION_H

#define SELECTION_H

#include <QObject>
#include "PhotoDB.h"
#include <QDateTime>
#include "Strip.h"
#include <QSet>

class Selection: public QObject {
  Q_OBJECT;
public:
  Selection(PhotoDB const &photodb, QObject *parent=0);
  int count();
  QSet<quint64> current();
public slots:
  void add(quint64 vsn);
  void addDateRange(QDateTime d1, QDateTime inclusiveend);
  void addDateRange(QDateTime start, Strip::TimeScale scl);
  void selectAll(); // current impl. doesn't know about filters
  // current impl. doesn't know about filters
  void remove(quint64 vsn);
  void clear();
  bool contains(quint64 vsn);
private:
  PhotoDB db;
};

#endif
