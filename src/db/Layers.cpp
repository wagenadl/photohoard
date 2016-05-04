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
  QSqlQuery q = db->constQuery("select id, stacking, active, typ, dat"
			       " from layers where version==:a"
			       " order by stacking", vsn);
  int expectedord = 0;
  while (q.next()) {
    expectedord++;
    Layer l;
    l.id = q.value(0).toULongLong();
    int ord = q.value(1).toInt();
    ASSERT(ord==expectedord);
    l.active = q.value(2).toBool();
    l.type = Layer::Type(q.value(3).toInt());
    l.data = q.value(4).toByteArray();
    layers << l;
  }
}

int Layers::size() const {
  return layers.size();
}

Layer const &Layers::layer(int n) const {
  ASSERT(n>=1 && n<=size());
  return layers[n-1];
}

template <class X> void autoTrans(X foo, Database *db, bool notrans) {
  if (notrans) {
    Untransaction t(db);
    foo();
  } else {
    Transaction t(db);
    foo();
    t.commit();
  }    
}  

void Layers::addLayer(Layer const &l, bool notrans) {
  layers << l;
  COMPLAIN("write to db!");
}

void Layers::raiseLayer(int n, bool notrans) {
  ASSERT(n>=1 && n<=size());
  if (n==size())
    return;
  Layer l0 = layers[n-1];
  Layer l1 = layers[n];
  layers[n-1] = l1;
  layers[n] = l0;
  COMPLAIN("Write to db!");
}

void Layers::lowerLayer(int n, bool notrans) {
  ASSERT(n>=1 && n<=size());
  if (n==1)
    return;
  Layer l0 = layers[n-1];
  Layer l1 = layers[n-2];
  layers[n-1] = l1;
  layers[n-2] = l0;
  COMPLAIN("Write to db!");
}

void Layers::deleteLayer(int n, bool notrans) {
  ASSERT(n>=1 && n<=size());
  layers.removeAt(n-1);
  COMPLAIN("Write to db!");
}

void Layers::setLayer(int n, Layer const &l, bool notrans) {
  ASSERT(n>=1 && n<=size());
  layers[n-1] = l;
  auto foo = [=]() {
    db->query("update layers(active,typ,dat) values(:a,:b,:c)"
	      " where version==:d and stacking==:e",
	      l.active, int(l.type), l.data,
	      vsn, n);
  };
  autoTrans(foo, db, notrans);
}
  
