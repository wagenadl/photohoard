// CanonLenses.h

#ifndef CANONLENSES_H

#define CANONLENSES_H

#include <QMultiMap>

class CanonLenses {
public:
  CanonLenses();
  ~CanonLenses();
  QString operator[](quint64) const;
  bool contains(quint64 id) const;
  static quint64 buildID(int lenstype, int f_low, int f_high, int f_div);
private:
  QMultiMap<quint64, QString> lenses;
private:
  CanonLenses(CanonLenses const &) = delete;
  CanonLenses &operator=(CanonLenses const &) = delete;
};

#endif
