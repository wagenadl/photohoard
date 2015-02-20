// Sliders.h

#ifndef SLIDERS_H

#define SLIDERS_H

#include <QMap>
#include <QString>
#include <QList>

class Sliders {
public:
  Sliders();
  void setAll(QString kv);
  QString getAll() const;
  bool set(QString k, double v);
  double get(QString k) const;
  QList<QString> list() const;
public:
#define SLIDER(name, dfl) double name;
#include "sliders.def"
#undef SLIDER
  static QMap<QString, double> const &defaults();
};

#endif
