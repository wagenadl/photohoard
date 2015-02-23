// Sliders.h

#ifndef SLIDERS_H

#define SLIDERS_H

#include <QMap>
#include <QStringList>

class Sliders {
public:
  Sliders();
  void setAll(QString kv);
  QString getAll() const;
  bool set(QString k, double v);
  double get(QString k) const;
  bool operator==(Sliders const &) const;
  bool couldBeAncestorOf(Sliders const &s) const;
  /* Returns true if for all of our sliders the value is either the default
     or the same as that in S. Note that this isn't a strict guarantee that 
     an image with settings S can be produced from an image with our settings:
     There are some sliders that the Adjuster can only apply in pairs or
     groups, and we are ignorant of that.
  */
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
