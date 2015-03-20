// Tags.cpp

#include "Tags.h"
#include <QStringList>

Tags::Tags(PhotoDB const &db): db(db) {
}

QString Tags::name(int id) {
  return db.simpleQuery("select tag from tags where id==:a", id).toString();
}

int Tags::findOne(QString tag) {
  QSqlQuery q = db.query("select id from tags where tag==:a", tag);
  int id = 0;
  if (q.next())
    id = q.value(0).toInt();
  if (q.next())
    id = -1;
  return id;
}

QSet<int> Tags::findAll(QString tag) {
  QSqlQuery q = db.query("select id from tags where tag==:a", tag);
  QSet<int> ids;
  while (q.next())
    ids << q.value(0).toInt();
  return ids;
}

QSet<int> Tags::findAbbreviated(QString tag) {
  QSqlQuery q = db.query("select id from tags where tag like \""
			 + tag + "%\"");
  QSet<int> ids;
  while (q.next())
    ids << q.value(0).toInt();
  return ids;
}

QSet<int> Tags::children(int tagid) {
  QSet<int> ids;
  QSqlQuery q = db.query("select id from tags where parent==:a", tagid);
  while (q.next())
    ids << q.value(0).toInt();
  ids.remove(0); // avoid loop
  return ids;
}

QSet<int> Tags::descendants(int tagid) {
  QSet<int> cc = children(tagid);
  QSet<int> res = cc;
  for (int child: cc)
    res.unite(descendants(child));
  return res;
}

QSet<int> Tags::applied(quint64 versionid) {
  QSet<int> res;
  QSqlQuery q = db.query("select tag from appliedtags where version==:a",
			 versionid);
  while (q.next())
    res << q.value(0).toInt();
  return res;
}

void Tags::apply(quint64 versionid, int tagid) {
  db.query("insert into appliedtags(version, tag) values(:a,:b)",
	   versionid, tagid);
}

void Tags::remove(quint64 versionid, int tagid) {
  db.query("delete from appliedtags where version==:a and tag==:b",
	   versionid, tagid);
}

int Tags::define(QString tag, int parent) {
  while (tag.endsWith(".") || tag.endsWith(" "))
    tag = tag.left(tag.size()-1);
  QSqlQuery q = db.query("insert into tags(tag,parent) values(:a,:b)",
			 tag, parent);
  int tagid = q.lastInsertId().toInt();
  return tagid;
}

bool Tags::undefine(int tagid) {
  if (db.simpleQuery("select count(*) from appliedtags where tag==:a",
		     tagid).toInt()>0)
    return false;
  QSet<int> cc = descendants(tagid);
  for (int c: cc)
    if (db.simpleQuery("select count(*) from appliedtags where tag==:a",
		       c).toInt()>0)
      return false;
  db.query("delete from tags where id==:a", tagid);
  return true;
}

int Tags::parent(int tagid) {
  return db.simpleQuery("select parent from tags where id==:a", tagid).toInt();
}

QString Tags::fullName(int tagid) {
  QStringList bits;
  while (tagid) {
    bits.prepend(name(tagid));
    tagid = parent(tagid);
  }
  return bits.join(":");
}

QString Tags::smartName(int tagid) {
  QString n = name(tagid);
  QStringList bits;
  bits << n;
  QSet<int> sibs = findAll(n);
  while (true) {
    if (sibs.size()==1)
      return bits.join(":");
    int pid = parent(tagid);
    if (pid==0)
      return ":" + bits.join(":");
    QString pn = name(pid);
    bits.prepend(pn);
    QSet<int> psibs = findAll(pn);
    bool ok = true;
    for (int s: psibs) {
      if (s!=pid && name(s)==pn) {
	ok = false;
	break;
      }
    }
    if (ok)
      return bits.join(":");
    tagid = pid;
    n = pn;
    sibs = psibs;
  }
}

int Tags::smartFind(QString tagl) {
  QSet<int> ids = smartFindAll(tagl);
  if (ids.isEmpty())
    return 0;
  else if (ids.size()==1)
    return *ids.begin();
  else
    return -1;
}

QSet<int> Tags::smartFindAll(QString tagl) {
  if (tagl.contains("::")) {
    QStringList bits = tagl.split("::");
    if (bits.size()==2)
      return ancestorfind(bits[0], bits[1]);
    else
      return QSet<int>();
  } else {
    QStringList bits = tagl.split(":");
    return chainfind(bits);
  }
}

QSet<int> Tags::ancestorfind(QString anc, QString dec) {
  QSet<int> posdec = findAllOrAbbreviated(dec);
  if (posdec.isEmpty())
    return QSet<int>();
  
  QSet<int> posanc = findAllOrAbbreviated(anc);
  if (posanc.isEmpty())
    return QSet<int>();

  QSet<int> res;
  for (int a: posanc) 
    res |= descendants(a).intersect(posdec);

  return res;
}

QSet<int> Tags::findAllOrAbbreviated(QString tag) {
  if (tag.endsWith(".") || tag.endsWith(" ")) 
    return findAll(tag.left(tag.size()-1));
  
  QSet<int> res = findAll(tag);
  if (res.isEmpty())
    res = findAbbreviated(tag);
  return res;
}

QSet<int> Tags::chainfind(QStringList bits) {
  if (bits.isEmpty())
    return QSet<int>();
  QSet<int> cc = findAllOrAbbreviated(bits.takeLast());
  if (bits.isEmpty())
    return cc;
    
  QSet<int> pp = chainfind(bits);
  if (pp.isEmpty())
    return QSet<int>();
  QSet<int> res;
  for (int c: cc) 
    if (pp.contains(parent(c)))
      res << c;
  return res;
}

bool Tags::couldBeNew(QString tag) {
  if (tag.contains("::"))
    return false;
  QStringList bits = tag.split(":");
  QString leaf = bits.takeLast();
  if (bits.isEmpty()) 
    return true;

  QString parent = bits.join(":");
  int n = smartFindAll(parent).size();
  if (n==1)
    return true;
  else if (n==0)
    return couldBeNew(parent);
  else
    return 0;
}

int Tags::define(QString tag) {
  if (!couldBeNew(tag))
    return 0;
  QStringList bits = tag.split(":");
  QString leaf = bits.takeLast();
  if (bits.isEmpty())
    return define(leaf, 0);
  int parent = smartFind(bits.join(":"));
  if (parent==0)
    parent = define(bits.join(":"));
  if (parent>0)
    return define(leaf, parent);
  else
    return 0;
}
 
