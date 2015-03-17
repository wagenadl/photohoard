// NikonLenses.cpp

#include "NikonLenses.h"
#include "PDebug.h"


NikonLenses::NikonLenses() {
#include "../res/fmountlens.h"
}

NikonLenses::~NikonLenses() {
}

QString NikonLenses::operator[](quint64 id) const {
  if (contains(id))
    return lenses.find(id)->trimmed();
  else
    return QString();
}

bool NikonLenses::contains(quint64 id) const {
  return lenses.contains(id);
}
