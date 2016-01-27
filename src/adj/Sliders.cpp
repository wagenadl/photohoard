// Sliders.cpp

// g++ -I/usr/share/qt4/include -I/usr/share/qt4/include/QtCore -E Sliders.cpp

#include "Sliders.h"
#include "math.h"
#include "PDebug.h"
#include <QStringList>
#include <limits>
#include "PhotoDB.h"
#include <QMetaType>

class SlidersFoo {
public:
  SlidersFoo() {
    qRegisterMetaType<Sliders>("Sliders");
  }
};

static SlidersFoo foo;


Sliders::Sliders() {
  reset();
}

bool Sliders::set(QString k, double v) {
#define SLIDER(name, dfl) if (k==#name) { name = v; return true; }
#include "sliders.def"
#undef SLIDER
  return false;
}

double Sliders::get(QString k) const {
#define SLIDER(name, dfl) if (k==#name) { return name; }
#include "sliders.def"
#undef SLIDER
  return std::numeric_limits<double>::quiet_NaN();
}

QMap<QString, double> const &Sliders::defaults() {
  static QMap<QString, double> df;
  if (df.isEmpty()) {
#define SLIDER(name, dfl) df[#name] = dfl;
#include "sliders.def"
#undef SLIDER
  }
  return df;
}

QStringList const &Sliders::keys() {
  static QStringList kk;
  if (kk.isEmpty()) {
#define SLIDER(name, dfl) kk << #name;
#include "sliders.def"
#undef SLIDER
  }
  return kk;
}

void Sliders::reset() {
#define SLIDER(name, dfl) name = dfl;
#include "sliders.def"
#undef SLIDER
}

void Sliders::setAll(QString kvv) {
  reset();
  QStringList pairs = kvv.split(";");
  for (QString pair: pairs) {
    QStringList kv = pair.split("=");
    if (kv.size()==2) {
      QString k = kv[0].simplified();
      QString v = kv[1].simplified();
      set(k, v.toDouble()); // could be more sophisticated
    } else if (!pair.isEmpty()) {
      COMPLAIN("Sliders: Bad kv pair ");
    }
  }
}

QString Sliders::getAll() const {
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

bool Sliders::operator==(Sliders const &s) const {
  return true
#define SLIDER(name, dfl) && name==s.name
#include "sliders.def"
#undef SLIDER
    ;
}

bool Sliders::isDefault() const {
  return true
#define SLIDER(name, dfl) && name==name##Default
#include "sliders.def"
#undef SLIDER
    ;
}

bool Sliders::isDefault(QString k) const {
  return get(k) == defaultFor(k);
}

Sliders Sliders::fromDB(quint64 vsn, class PhotoDB &db) {
  Sliders s;
  s.readFromDB(vsn, db);
  return s;
}

void Sliders::readFromDB(quint64 vsn, PhotoDB &db) {
  reset();
  QSqlQuery q = db.query("select k, v from adjustments where version==:a",
                         vsn);
  while (q.next())
    set(q.value(0).toString(), q.value(1).toDouble());
}
  
void Sliders::writeToDB(quint64 vsn, PhotoDB &db) const {
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

PSize Sliders::cropSize(PSize filesize, Exif::Orientation orient) {
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
