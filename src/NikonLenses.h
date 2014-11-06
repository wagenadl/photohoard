// NikonLenses.h

#ifndef NIKONLENSES_H

#define NIKONLENSES_H

#include <QMultiMap>

class NikonLenses {
public:
  NikonLenses();
  ~NikonLenses();
  QString operator[](quint64) const;
  bool contains(quint64 id) const;
private:
  QMultiMap<quint64, QString> lenses;
private:
  NikonLenses(NikonLenses const &) = delete;
  NikonLenses &operator=(NikonLenses const &) = delete;
};

#endif
