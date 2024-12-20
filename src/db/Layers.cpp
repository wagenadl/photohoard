// Layers.cpp

#include "Layers.h"
#include "PDebug.h"
#include "PhotoDB.h"
#include <QPainter>
#include <QLinearGradient>
#include "Adjuster.h"
#include "Geometry.h"
#include <cmath>
#include "Spline.h"
#include "PhotoOps.h"

QString Layer::typeName(Layer::Type t) {
  switch (t) {
  case Type::Invalid:
    return "Invalid";
  case Type::Base:
    return "(Base)";
  case Type::LinearGradient:
    return "Linear gradient";
    //case Type::Circular:
    //return "Circular area";
  case Type::Curve:
    return "Curved edge";
  case Type::Area:
    return "Curved area";
  case Type::Clone:
    return "Clone";
  case Type::Inpaint:
    return "Heal";
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

QList<int> Layer::radii() const {
  char const *raw = dat.data();
  qint16 const *pts = reinterpret_cast<qint16 const *>(raw);
  int Nint = dat.size() / 2;
  int N0 = points().size() * 2;

  QList<int> rr;
  for (int n=N0; n<Nint; n++)
    rr << pts[n];
  return rr;
}

QPolygon Layer::points() const {
  char const *raw = dat.data();
  qint16 const *pts = reinterpret_cast<qint16 const *>(raw);
  int Nint = dat.size() / 2;
  int N = 0; // number of points
  switch (typ) {
  case Type::Invalid: N = 0; break;
  case Type::Base: N = 0; break;
  case Type::LinearGradient: N = 2; break;
  // case Type::Circular: N = 1; break;
  case Type::Curve: N = (Nint-1) / 2; break;
  case Type::Area: N = (Nint-1) / 2; break;
  case Type::Clone: N = 2 * Nint / 5; break;
  case Type::Inpaint: N = Nint / 3; break;
  }
  QPolygon p(N);
  for (int n=0; n<N; n++) {
    p.setPoint(n, pts[0], pts[1]);
    pts += 2;
  }
  return p;
}

void Layer::setPointsAndRadii(QPolygon const &p, QList<int> const &rr) {
  int N = p.size();
  int M = rr.size();
  dat.resize(4*N + 2*M);
  char *raw = dat.data();
  qint16 *pts = reinterpret_cast<qint16 *>(raw);
  for (int n=0; n<N; n++) {
    QPoint p0 = p[n];
    *pts++ = p0.x();
    *pts++ = p0.y();
  }
  for (int m=0; m<M; m++)
    *pts++ = rr[m];
}

double Layer::scale(QSize osize, class Adjustments const &adj0,
                    QSize sclcrpsize) const {
  /* I know the original size and the scaled cropped size. Can I figure
     out the scale factor? Of course: I just need to find the unscaled
     cropped size first
  */
  PSize crpsize = Geometry::croppedSize(osize, adj0);
  double xscale = sclcrpsize.width()*1.0/crpsize.width();
  double yscale = sclcrpsize.height()*1.0/crpsize.height();
  return std::sqrt(xscale*yscale);
}

QPolygonF Layer::transformedPoints(QSize osize, class Adjustments const &adj0,
                                   QSize sclcrpsize) const {
  double scl = scale(osize, adj0, sclcrpsize);
  return Geometry::mapToScaledAdjusted(points(), osize, adj0, scl);
}

QList<double> Layer::transformedRadii(QSize osize,
                                      class Adjustments const &adj0,
                                      QSize sclcrpsize) const {
  double scl = scale(osize, adj0, sclcrpsize);
  QList<double> radi;
  for (int r: radii())
    radi << scl * r;
  return radi;
}

QImage Layer::mask(QSize osize, class Adjustments const &adj0,
		   QSize sclcrpsize) const {
  QImage msk(sclcrpsize, QImage::Format_Grayscale8);
  if (!active) {
    msk.fill(0);
    return msk;
  }
  if (sclcrpsize.isEmpty()) {
    qDebug () << "Layer::mask: empty request";
    return msk;
  }
  QPolygonF pts(transformedPoints(osize, adj0, sclcrpsize));
  QList<double> radi = transformedRadii(osize, adj0, sclcrpsize);
  switch (typ) {
  case Type::Invalid:
    msk.fill(0);
    break;
  case Type::Base:
    msk.fill(255);
    break;
  case Type::LinearGradient: {
    ASSERT(pts.size()==2);
    QPainter ptr(&msk);
    QLinearGradient gr(pts[0], pts[1]);
    gr.setColorAt(0, QColor(0,0,0));
    gr.setColorAt(1, QColor(255,255,255));
    ptr.fillRect(QRect(QPoint(0,0), msk.size()), gr);
  } break;
  case Type::Area: {
    msk.fill(0);
    QPolygonF ppp = Spline(pts, 2).points;
    { QPainter ptr(&msk);
      ptr.setPen(QPen(Qt::NoPen));
      if (!ptr.isActive())
        qDebug() << "Painter not active in Layers.cpp";
      ptr.setBrush(QBrush(QColor(255,255,255)));
      ptr.drawPolygon(ppp);
    }
    if (radii().size()!=1) {
      pDebug() << "Number of radii" << radii().size() << "for layer type Area";
      break;
    }
    PhotoOps::blur(msk, radi[0]);
  } break;
  //case Type::Circular:
  case Type::Curve:
    msk.fill(0);
    COMPLAIN("layer mask NYI");
    break;
  case Type::Clone:
  case Type::Inpaint:
    msk.fill(0);
    { QPainter ptr(&msk);
      ptr.setPen(QPen(Qt::NoPen));
      if (!ptr.isActive())
        qDebug() << "Painter not active in Layers.cpp";
      ptr.setBrush(QBrush(QColor(255,255,255)));
      for (int k=0; k<radi.size(); k++) 
        ptr.drawEllipse(pts[k], radi[k], radi[k]);
    }
    if (typ==Type::Clone)
      PhotoOps::blur(msk, radi[0]/10);
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
  DBReadLock lock(&db);
  QSqlQuery q = db.constQuery("select active, typ, alpha, name, dat"
			      " from layers where id==:a",
			      layerid);
  readFromDB(q);
}

void Layer::readFromDB(quint64 vsn, int stacking, PhotoDB const &db) {
  DBReadLock lock(&db);
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
  DBWriteLock lock(&db);
  pDebug() << "Layer::writetodb";
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
  DBReadLock lock(db);
  return db->simpleQuery("select count(1) from layers"
			 " where version==:a", vsn).toInt();
}

quint64 Layers::layerID(int n) const {
  ASSERT(n>=1 && n<=count());
  DBReadLock lock(db);
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
  pDebug() << "layerstrans1";
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
  pDebug() << "layerstrans2";
  quint64 idn = layerID(n);
  quint64 idbelow = layerID(n-1);
  db->query("update layers set stacking=:a where id==:b", n-1, idn);
  db->query("update layers set stacking=:a where id==:b", n, idbelow);
  t.commit();
}

void Layers::deleteLayer(int n) {
  int N = count();
  pDebug() << "deletelayer" << n << N;
  ASSERT(n>=1 && n<=N);
  Transaction t(db);
  pDebug() << "layerstrans3";
  db->query("delete from layers where version==:a and stacking==:b",
	    vsn, n);
  for (int k=n+1; k<=N; k++)
    db->query("update layers set stacking=:a"
	      " where version==:b and stacking==:c", k-1, vsn, k);
  t.commit();
}

void Layers::setLayer(int n, Layer const &l) {
  ASSERT(n>=1 && n<=count());
  l.writeToDB(vsn, n, *db);
}
