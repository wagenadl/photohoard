// Sliders.h

#ifndef SLIDERS_H

#define SLIDERS_H

#include <QMap>
#include <QStringList>

class Sliders {
public:
  Sliders();
  explicit Sliders(QString kv): Sliders() { setAll(kv); }
  void setAll(QString kv);
  QString getAll() const;
  bool set(QString k, double v);
  double get(QString k) const;
  bool operator==(Sliders const &) const;
  bool isDefault() const;
  bool isDefault(QString k) const;
public:
#define SLIDER(name, dfl) double name;
#include "sliders.def"
#undef SLIDER
#define SLIDER(name, dfl) static constexpr double name##Default = dfl;
#include "sliders.def"
#undef SLIDER
  static QStringList const &keys();
  static QMap<QString, double> const &defaults();
  static double defaultFor(QString k) { return defaults()[k]; }
};

#endif
