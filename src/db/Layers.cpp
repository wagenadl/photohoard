// Layers.cpp

#include "Layers.h"
#include "PDebug.h"
#include "PhotoDB.h"
#include <QPainter>
#include <QLinearGradient>
#include "Adjuster.h"
#include "Geometry.h"
#include <cmath>

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
  alph = 1;
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

double Layer::alpha() const {
  return alph;
}

void Layer::setAlpha(double a) {
  alph = a;
}

QString Layer::name() const {
  return name_;
}

void Layer::setName(QString s) {
  name_ = s;
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

QImage Layer::mask(QSize osize, class Adjustments const &adj0,
		   QSize sclcrpsize) const {
  /* I know the original size and the scaled cropped size. Can I figure
     out the scale factor? Of course: I just need to find the unscaled
     cropped size first
  */
  PSize crpsize = Geometry::croppedSize(osize, adj0);
  double xscale = sclcrpsize.width()*1.0/crpsize.width();
  double yscale = sclcrpsize.height()*1.0/crpsize.height();
  double scale = std::sqrt(xscale*yscale);
  /* So now I have the scale factor needed for coordinate mapping.
   */
  QImage msk(sclcrpsize, QImage::Format_Grayscale8);
  switch (typ) {
  case Type::Invalid:
    msk.fill(0);
    break;
  case Type::Base:
    msk.fill(255);
    break;
  case Type::LinearGradient: {
    QPolygonF pts(Geometry::mapToScaledAdjusted(points(),
						osize, adj0, scale));
    ASSERT(pts.size()==2);
    QPainter ptr(&msk);
    QLinearGradient gr(pts[0], pts[1]);
    gr.setColorAt(0, QColor(0,0,0));
    gr.setColorAt(1, QColor(255,255,255));
    ptr.fillRect(QRect(QPoint(0,0), msk.size()), gr);
  } break;
  case Type::Circular:
  case Type::Curve:
  case Type::Area:
    COMPLAIN("layer mask NYI");
    break;
  }
  return msk;
}

Layer Layer::fromDB(quint64 layerid, PhotoDB const &db) {
  Layer l;
  l.readFromDB(layerid, db);
  return l;
}

Layer Layer::fromDB(quint64 vsn, int stacking, PhotoDB const &db) {
  Layer l;
  l.readFromDB(vsn, stacking, db);
  return l;
}

void Layer::readFromDB(quint64 layerid, PhotoDB const &db) {
  QSqlQuery q = db.constQuery("select active, typ, alpha, name, dat"
			      " from layers where id==:a",
			      layerid);
  readFromDB(q);
}

void Layer::readFromDB(quint64 vsn, int stacking, PhotoDB const &db) {
  QSqlQuery q = db.constQuery("select active, typ, alpha, name, dat"
			      " from layers where version==:a"
			      " and stacking==:b",
			      vsn, stacking);
  readFromDB(q);
}

void Layer::readFromDB(QSqlQuery &q) {
  if (!q.next()) 
    CRASH("LAYER NOT FOUND");
  setActive(q.value(0).toBool());
  setType(Layer::Type(q.value(1).toInt()));
  setAlpha(q.value(2).toDouble());
  setName(q.value(3).toString());
  setData(q.value(4).toByteArray());
}

void Layer::writeToDB(quint64 vsn, int stacking, PhotoDB &db) const {
  QSqlQuery q = db.constQuery("select id from layers"
			      " where version==:a and stacking==:b",
			      vsn, stacking);
  if (q.next()) {
    quint64 layerid = q.value(0).toULongLong();
    db.query("update layers set active=:a, typ=:b, alpha=:c, name=:d, dat=:e"
	     " where id==:f",
	     isActive(), int(type()), alpha(), name(), data(),
	     layerid);
  } else {
    db.query("insert into layers (version, stacking,"
	     " active, typ, alpha, name, dat)"
	     " values (:a, :b, :c, :d, :e, :f, :g)",
	     vsn, stacking,
	     isActive(), int(type()), alpha(), name(), data());
  }
}

bool Layer::operator==(Layer const &l) const {
  return typ==l.typ
    && active==l.active
    && alph==l.alph
    && dat==l.dat;
}

//////////////////////////////////////////////////////////////////////

Layers::Layers(quint64 vsn, PhotoDB *db): db(db), vsn(vsn) {
}

int Layers::count() const {
  return db->simpleQuery("select count(1) from layers"
			 " where version==:a", vsn).toInt();
}

quint64 Layers::layerID(int n) const {
  ASSERT(n>=1 && n<=count());
  return  db->simpleQuery("select id from layers"
			  " where version==:a and stacking==:b",
			  vsn, n).toULongLong();
}

Layer Layers::layer(int n) const {
  ASSERT(n>=1 && n<=count());
  return Layer::fromDB(vsn, n, *db);
}


void Layers::addLayer(Layer const &l) {
  int N = count();
  l.writeToDB(vsn, N+1, *db);
}

void Layers::raiseLayer(int n) {
  int N = count();
  ASSERT(n>=1 && n<=N);
  if (n==N)
    return;
  Transaction t(db);
  quint64 idn = layerID(n);
  quint64 idabove = layerID(n+1);
  db->query("update layers set stacking=:a where id==:b", n+1, idn);
  db->query("update layers set stacking=:a where id==:b", n, idabove);
  t.commit();
}

void Layers::lowerLayer(int n) {
  ASSERT(n>=1 && n<=count());
  if (n==1)
    return;
  Transaction t(db);
  quint64 idn = layerID(n);
  quint64 idbelow = layerID(n-1);
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
  l.writeToDB(vsn, n, *db);
}
