// CanonLenses.cpp

#include "CanonLenses.h"
#include "PDebug.h"


CanonLenses::CanonLenses() {
#include "../res/efmountlens.h"
}

CanonLenses::~CanonLenses() {
}

QString CanonLenses::operator[](quint64 id) const {
  if (id & 0x8000) {
    int f_low = id >> 16;
    int f_high = f_low >> 12;
    f_low &= 0xfff;
    if (f_low==f_high)
      return QString("Unknown %1mm").arg(f_low);
    else
      return QString("Unknown %1-%2mm").arg(f_low).arg(f_high);
  }
  
  if (contains(id))
    return lenses.find(id)->trimmed();
  else
    return QString();
}

bool CanonLenses::contains(quint64 id) const {
  pDebug() << "CanonLenses::contains?" << QString("0x%1").arg(id, 10, 16, QChar('0'));
  if (id & 0x8000)
    return true; // negative, will confabulate
  else return lenses.contains(id);
}

quint64 CanonLenses::buildID(int lenstype, int f_low, int f_high,
                                    int f_div) {
  if (f_div>0) {
    f_low /= f_div;
    f_high /= f_div;
  }
  constexpr quint64 mul = 65536;
  return (lenstype & 0xffff) + mul*(f_low + 4096*f_high);
}
