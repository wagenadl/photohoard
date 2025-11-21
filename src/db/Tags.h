// Tags.h

#ifndef TAGS_H

#define TAGS_H

#include "PhotoDB.h"

class Tags {
public:
  Tags(PhotoDB *db);
public:
  // these create readlocks
  int collectionRoot();
  QStringList collections();
  int findCollection(QString tag); // name must be exact apart from case
  int findOne(QString tag); // 0 for not found or -1 if not unique
  QSet<int> findAll(QString tag); // find all tags matching name (exact apart from case; includes collections)
  QSet<int> findAbbreviated(QString tag);
  int find(QString tag, int parent);
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
  bool canUndefine(int);
  QSet<int> applied(quint64 versionid);
  bool couldBeNew(QString tag);
  QString interpretation(QStringList ss);
  // new style
  QStringList findTagsStartingWith(QString partialtag, int limit=-1);
  QStringList findTagsContaining(QString partialtag, limit=-1);
public: // these create transactions/write locks
  int ensureCollection(QString); // creates if not existing
  int define(QString tag, int parent);
  int define(QString tagl); // returns 0 if not definable
  bool undefine(int); // returns true if OK, fails if applied or parent.
  void apply(quint64 versionid, int tagid);
  void apply(QSet<quint64> const &versionid, int tagid);
  void remove(quint64 versionid, int tagid);
  void remove(QSet<quint64> const &versionid, int tagid);
public:
  static QString normalCase(QString);
private:
  QSet<int> ancestorfind(QString anc, QString dec);
  QSet<int> chainfind(QStringList bits);
  QSet<int> findAllOrAbbreviated(QString tag);
private:
  PhotoDB *db;
};

#endif
