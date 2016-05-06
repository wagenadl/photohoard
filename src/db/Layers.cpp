// Layers.cpp

#include "Layers.h"
#include "PDebug.h"
#include "PhotoDB.h"

Layer::Layer() {
  id = 0;
  active = false;
  type = Type::Invalid;
}

QPolygon Layer::points() const {
  char const *raw = data.data();
  quint16 const *pts = reinterpret_cast<quint16 const *>(raw);
  int N = data.size() / 4;
  QPolygon p(N);
  for (int n=0; n<N; n++) {
    p.setPoint(n, pts[0], pts[1]);
    pts += 2;
  }
  return p;
}

void Layer::setPoints(QPolygon const &p) {
  int N = p.size();
  data.resize(4*N);
  char *raw = data.data();
  quint16 *pts = reinterpret_cast<quint16 *>(raw);
  for (int n=0; n<N; n++) {
    QPoint p0 = p[n];
    *pts++ = p0.x();
    *pts++ = p0.y();
  }
}

QImage Layer::mask(QSize origSize, class Adjustments const &) const {
  COMPLAIN("NYI");
  return QImage();
}
  
Layers::Layers(quint64 vsn, PhotoDB *db): db(db), vsn(vsn) {
}

int Layers::size() const {
  return db->simpleQuery("select count(1)"
			 " from layers where version==:a"
			 " order by stacking", vsn).toInt();
}

Layer Layers::layer(int n) const {
  ASSERT(n>=1 && n<=size());

  QSqlQuery q = db->constQuery("select id, active, typ, dat"
			       " from layers where version==:a"
			       " and stacking==:b", vsn, n);
  if (!q.next()) 
    CRASH("LAYER NOT FOUND");

  Layer l;
  l.id = q.value(0).toULongLong();
  l.active = q.value(1).toBool();
  l.type = Layer::Type(q.value(2).toInt());
  l.data = q.value(3).toByteArray();
  return l;
}


void Layers::addLayer(Layer const &l) {
  int N = size();
  Transaction t(db);
  db->query("insert layers(version, stacking, active, typ, dat)"
	    " values(:a,:b,:c,:d,:e)",
	    vsn, N+1, l.active, int(l.type), l.data);
  t.commit();
  COMPLAIN("write to db!");
}

void Layers::raiseLayer(int n) {
  ASSERT(n>=1 && n<=size());
  if (n==size())
    return;
  Transaction t(db);
  quint64 idn = db->simpleQuery("select id from layers where version==:a"
				" and stacking==:b", vsn, n).toULongLong();
  quint64 idabove = db->simpleQuery("select id from layers where version==:a"
				    " and stacking==:b", vsn, n+1)
    .toULongLong();
  db->query("update layers set stacking=:a where id==:b", n+1, idn);
  db->query("update layers set stacking=:a where id==:b", n, idabove);
  t.commit();
}

void Layers::lowerLayer(int n) {
  ASSERT(n>=1 && n<=size());
  if (n==1)
    return;
  Transaction t(db);
  quint64 idn = db->simpleQuery("select id from layers where version==:a"
				" and stacking==:b", vsn, n).toULongLong();
  quint64 idbelow = db->simpleQuery("select id from layers where version==:a"
				    " and stacking==:b", vsn, n-1)
    .toULongLong();
  db->query("update layers set stacking=:a where id==:b", n-1, idn);
  db->query("update layers set stacking=:a where id==:b", n, idbelow);
  t.commit();
}

void Layers::deleteLayer(int n) {
  int N = size();
  ASSERT(n>=1 && n<=N);
  Transaction t(db);
  db->query("delete from layers where version==:a and stacking==:b",
	    vsn, n);
  for (int k=n+1; k<=N; k++)
    db->query("update layers set stacking=:a"
	      " where version==:b and stacking==:c", vsn, k-1, k);
  t.commit();
}

void Layers::setLayer(int n, Layer const &l) {
  ASSERT(n>=1 && n<=size());
  Transaction t(db);
  db->query("update layers set active=:a, typ=:b, dat=:c"
	    " where version==:d and stacking==:e",
	    l.active, int(l.type), l.data,
	    vsn, n);
  t.commit();
}
  
