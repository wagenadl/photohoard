// DateTime.cpp

#include "DateTime.h"

DateTime::DateTime() {
  ms = 0;
}

DateTime::DateTime(QDateTime const &dt): dt(dt) {
  ms = dt.toMSecsSinceEpoch();
}

DateTime::DateTime(qint64 ms): ms(ms) {
}

QDateTime const &DateTime::toQ() const {
  if (dt.isNull())
    dt = QDateTime::fromMSecsSinceEpoch(ms);
  return dt;
}

QString const &DateTime::toString() const {
  if (str.isNull()) {
    toQ();
    str = dt.toString(Qt::ISODate);
  }
  return str;
}

QString DateTime::toString(QString fmt) const {
  toQ();
  return dt.toString(fmt);
}


DateTime DateTime::addSeconds(int n) const {
  return DateTime(ms + 1000*n);
}

DateTime DateTime::addDays(int n) const {
  return DateTime(ms + 1000*24*60*60*n);
}

DateTime DateTime::addYears(int n) const {
  return DateTime(toQ().addYears(n));
}

DateTime DateTime::addMonths(int n) const {
  return DateTime(toQ().addMonths(n));
}
