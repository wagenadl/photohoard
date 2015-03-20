// Tags.h

#ifndef TAGS_H

#define TAGS_H

#include "PhotoDB.h"

class Tags {
public:
  Tags(PhotoDB const &db);
  int findOne(QString tag); // 0 for not found or -1 if not unique
  QSet<int> findAll(QString tag); // find all tags matching name
  QSet<int> findAbbreviated(QString tag);
  int smartFind(QString tagl); // finds things like lab::leech
  // or pla:cinc, allowing unique abbreviations.
  // returns 0 if not found, -1 if not unique
  QSet<int> smartFindAll(QString tagl); 
  QString name(int tagid);
  QString fullName(int tagid);
  QString smartName(int tagid);
  // returns a name this is unique, using parent:child as needed.
  QSet<int> descendants(int tagid);
  QSet<int> children(int tagid);
  int parent(int tagid);
  int define(QString tag, int parent);
  int define(QString tagl); // returns 0 if not definable
  bool undefine(int); // returns true if OK, fails if applied or parent.
  void apply(quint64 versionid, int tagid);
  void remove(quint64 versionid, int tagid);
  QSet<int> applied(quint64 versionid);
  bool couldBeNew(QString tag);
private:
  QSet<int> ancestorfind(QString anc, QString dec);
  QSet<int> chainfind(QStringList bits);
  QSet<int> findAllOrAbbreviated(QString tag);
private:
  PhotoDB db;
};

#endif
