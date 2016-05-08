// Adjustments.cpp

#include "Adjustments.h"
#include "math.h"
#include "PDebug.h"
#include <QStringList>
#include <limits>
#include "PhotoDB.h"
#include <QMetaType>

class AdjustmentsFoo {
public:
  AdjustmentsFoo() {
    qRegisterMetaType<Adjustments>("Adjustments");
  }
};

static AdjustmentsFoo foo;

Adjustments::Adjustments() {
  reset();
}

bool Adjustments::set(QString k, double v) {
#define ADJUSTMENT(name, dfl) if (k==#name) { name = v; return true; }
#include "AdjustmentDefs.h"
#undef ADJUSTMENT
  return false;
}

double Adjustments::get(QString k) const {
#define ADJUSTMENT(name, dfl) if (k==#name) { return name; }
#include "AdjustmentDefs.h"
#undef ADJUSTMENT
  return std::numeric_limits<double>::quiet_NaN();
}

QMap<QString, double> const &Adjustments::defaults() {
  static QMap<QString, double> df;
  if (df.isEmpty()) {
#define ADJUSTMENT(name, dfl) df[#name] = dfl;
#include "AdjustmentDefs.h"
#undef ADJUSTMENT
  }
  return df;
}

QStringList const &Adjustments::keys() {
  static QStringList kk;
  if (kk.isEmpty()) {
#define ADJUSTMENT(name, dfl) kk << #name;
#include "AdjustmentDefs.h"
#undef ADJUSTMENT
  }
  return kk;
}

void Adjustments::reset() {
#define ADJUSTMENT(name, dfl) name = dfl;
#include "AdjustmentDefs.h"
#undef ADJUSTMENT
}

void Adjustments::setAll(QString kvv) {
  reset();
  QStringList pairs = kvv.split(";");
  for (QString pair: pairs) {
    QStringList kv = pair.split("=");
    if (kv.size()==2) {
      QString k = kv[0].simplified();
      QString v = kv[1].simplified();
      set(k, v.toDouble()); // could be more sophisticated
    } else if (!pair.isEmpty()) {
      COMPLAIN("Adjustments: Bad kv pair ");
    }
  }
}

QString Adjustments::getAll() const {
  QStringList pairs;
  for (auto it=defaults().begin(); it!=defaults().end(); ++it) {
    QString k = it.key();
    double v0 = it.value();
    double v = get(k);
    if (v!=v0) 
      pairs << (k + "=" + QString::number(v, 'f', 2));
  }
  return pairs.join(";");
}

bool Adjustments::operator==(Adjustments const &s) const {
  return true
#define ADJUSTMENT(name, dfl) && name==s.name
#include "AdjustmentDefs.h"
#undef ADJUSTMENT
    ;
}

bool Adjustments::isDefault() const {
  return true
#define ADJUSTMENT(name, dfl) && name==name##Default
#include "AdjustmentDefs.h"
#undef ADJUSTMENT
    ;
}

bool Adjustments::isDefault(QString k) const {
  return get(k) == defaultFor(k);
}

Adjustments Adjustments::fromDB(quint64 vsn, class PhotoDB &db) {
  Adjustments s;
  s.readFromDB(vsn, db);
  return s;
}

Adjustments Adjustments::fromDB(quint64 vsn, int layer, class PhotoDB &db) {
  Adjustments s;
  s.readFromDB(vsn, layer, db);
  return s;
}

void Adjustments::readFromDB(quint64 vsn, PhotoDB &db) {
  reset();
  QSqlQuery q = db.query("select k, v from adjustments where version==:a",
                         vsn);
  while (q.next())
    set(q.value(0).toString(), q.value(1).toDouble());
}

void Adjustments::readFromDB(quint64 vsn, int layer, PhotoDB &db) {
  reset();
  quint64 layerid = db.simpleQuery("select id from layers"
				   " where version==:a and stacking==:b",
				   vsn, layer).toULongLong();
  QSqlQuery q = db.query("select k, v from layeradjustments where layer==:a",
                         layerid);
  while (q.next())
    set(q.value(0).toString(), q.value(1).toDouble());
}

void Adjustments::writeToDB(quint64 vsn, PhotoDB &db) const {
  for (auto it=defaults().begin(); it!=defaults().end(); ++it) {
    QString k = it.key();
    double v0 = it.value();
    double v = get(k);
    if (v==v0)
      db.query("delete from adjustments where version==:a and k==:b",
               vsn, k);
    else
      db.query("insert or replace into adjustments(version, k, v)"
               " values(:a,:b,:c)", vsn, k, v);
  }
}  

void Adjustments::writeToDB(quint64 vsn, int layer, PhotoDB &db) const {
  quint64 layerid = db.simpleQuery("select id from layers"
				   " where version==:a and stacking==:b",
				   vsn, layer).toULongLong();
  for (auto it=defaults().begin(); it!=defaults().end(); ++it) {
    QString k = it.key();
    double v0 = it.value();
    double v = get(k);
    if (v==v0)
      db.query("delete from layeradjustments where layer==:a and k==:b",
               layerid, k);
    else
      db.query("insert or replace into layeradjustments(layer, k, v)"
               " values(:a,:b,:c)", layerid, k, v);
  }
}  

PSize Adjustments::cropSize(PSize filesize, Exif::Orientation orient) {
  PSize size = filesize;
  switch (orient) {
  case Exif::Upright:
  case Exif::UpsideDown:
    break;
  case Exif::CCW:
  case Exif::CW:
    size.rotate90();
    break;
  }
  return PSize(size.width() - cropl - cropr,
	       size.height() - cropt - cropb);
}
