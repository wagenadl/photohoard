// Adjustment.cpp

#include "Adjustment.h"

Adjustment *Adjustment::create(QString typ, QList<double> const &args) {
  if (!creators().contains(typ))
    return NULL;
  Adjustment *a = creators()[typ]();
  if (!a)
    return NULL;

  a->setType(typ);
  a->setArguments(args);
  return a;
}

Adjustment *Adjustment::create(QString defn) {
  defn = defn.trimmed();
  if (defn.contains(";")) {
    return AdjustmentStack::create(defn.split(QRegExp(";")));
  } else {
    QStringList bits = defn.split(QRegExp("\\s+"));
    if (bits.isEmpty())
      return NULL;
    QString typ = bits.takeFirst();
    QList<double> args;
    for (auto s: bits)
      args << s.toDouble();
    return create(typ, args);
  }
}

// static Adjustment::Creator<Adjustment> c("null");

Adjustment::Adjustment() {
}

void Adjustment::setType(QString t) {
  typ = t;
}

void Adjustment::setArguments(QList<double> const &a) {
  args = a;
}
