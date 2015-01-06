// Selection.h

#ifndef SELECTION_H

#define SELECTION_H

#include <QObject>
#include "Database.h"
#include <QDateTime>

class Selection: public QObject {
  Q_OBJECT;
public:
  Selection(Database const &photodb, QObject *parent=0);
public slots:
  void add(quint64 vsn);
  void add(QDateTime start, QDateTime inclusiveend); // current impl. doesn't know about filters
  void remove(quint64 vsn);
  void clear();
  void selectAll(); // current impl. doesn't know about filters
  bool contains(quint64 vsn);
private:
  Database db;
};

#endif
