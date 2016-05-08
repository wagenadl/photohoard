// Layers.cpp

#include "Layers.h"
#include "PDebug.h"
#include "PhotoDB.h"

QString Layer::typeName(Layer::Type t) {
  switch (t) {
  case Type::Invalid:
    return "Invalid";
  case Type::Base:
    return "(Base)";
  case Type::LinearGradient:
    return "Linear gradient";
  case Type::Circular:
    return "Circular area";
  case Type::Curve:
    return "Curved edge";
  case Type::Area:
    return "Curved area";
  }
  return "Invalid"; // not reached
}

QString Layer::typeName() const {
  return typeName(typ);
}

Layer::Layer() {
  active = true;
  typ = Type::Invalid;
}

bool Layer::isActive() const {
  return active;
}

void Layer::setActive(bool a) {
  active = a;
}

Layer::Type Layer::type() const {
  return typ;
}

void Layer::setType(Type t) {
  typ = t;
}

QByteArray const &Layer::data() const {
  return dat;
}

void Layer::setData(QByteArray const &d) {
  dat = d;
}


QPolygon Layer::points() const {
  char const *raw = dat.data();
  quint16 const *pts = reinterpret_cast<quint16 const *>(raw);
  int N = dat.size() / 4;
  QPolygon p(N);
  for (int n=0; n<N; n++) {
    p.setPoint(n, pts[0], pts[1]);
    pts += 2;
  }
  return p;
}

void Layer::setPoints(QPolygon const &p) {
  int N = p.size();
  dat.resize(4*N);
  char *raw = dat.data();
  quint16 *pts = reinterpret_cast<quint16 *>(raw);
  for (int n=0; n<N; n++) {
    QPoint p0 = p[n];
    *pts++ = p0.x();
    *pts++ = p0.y();
  }
}

QImage Layer::mask(QSize, class Adjustments const &) const {
  COMPLAIN("NYI");
  return QImage();
}

//////////////////////////////////////////////////////////////////////

Layers::Layers(quint64 vsn, PhotoDB *db): db(db), vsn(vsn) {
}

int Layers::count() const {
  return db->simpleQuery("select count(1)"
			 " from layers where version==:a"
			 " order by stacking", vsn).toInt();
}

quint64 Layers::layerID(int n) const {
  return db->simpleQuery("select id from layers where versions==:a"
			 " and stacking==:b", vsn, n).toULongLong();
}

Layer Layers::layer(int n) const {
  ASSERT(n>=1 && n<=count());

  QSqlQuery q = db->constQuery("select id, active, typ, dat"
			       " from layers where version==:a"
			       " and stacking==:b", vsn, n);
  if (!q.next()) 
    CRASH("LAYER NOT FOUND");

  Layer l;
  l.setActive(q.value(1).toBool());
  l.setType(Layer::Type(q.value(2).toInt()));
  l.setData(q.value(3).toByteArray());
  return l;
}


void Layers::addLayer(Layer const &l) {
  int N = count();
  Transaction t(db);
  db->query("insert into layers(version, stacking, active, typ, dat)"
	    " values(:a,:b,:c,:d,:e)",
	    vsn, N+1, l.isActive(), int(l.type()), l.data());
  t.commit();
  COMPLAIN("write to db!");
}

void Layers::raiseLayer(int n) {
  int N = count();
  ASSERT(n>=1 && n<=N);
  if (n==N)
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
  ASSERT(n>=1 && n<=count());
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
  int N = count();
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
  ASSERT(n>=1 && n<=count());
  Transaction t(db);
  db->query("update layers set active=:a, typ=:b, dat=:c"
	    " where version==:d and stacking==:e",
	    l.isActive(), int(l.type()), l.data(),
	    vsn, n);
  t.commit();
}
  
