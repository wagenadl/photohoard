// Sliders.cpp

// g++ -I/usr/share/qt4/include -I/usr/share/qt4/include/QtCore -E Sliders.cpp

#include "Sliders.h"
#include "math.h"
#include <QDebug>
#include <QStringList>

Sliders::Sliders() {
#define SLIDER(name, dfl) name = dfl;
#include "sliders.def"
#undef SLIDER
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
  return nan("");
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

void Sliders::setAll(QString kvv) {
  QStringList pairs = kvv.split(";");
  for (QString pair: pairs) {
    QStringList kv = pair.split("=");
    if (kv.size()==2) {
      QString k = kv[0].simplified();
      QString v = kv[1].simplified();
      set(k, v.toDouble()); // could be more sophisticated
    } else {
      qDebug() << "Bad kv pair " << pair;
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
