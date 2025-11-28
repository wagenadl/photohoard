// DateTime.h

#ifndef DATETIME_H

#define DATETIME_H

#include <QDateTime>


class DateTime {
  static constexpr qint64 EONSAGO = -10000l * 365l * 24l * 3600l * 1000l;
public:
  DateTime();
  DateTime(QDateTime const &);
  explicit DateTime(qint64 ms);
  bool isValid() const { return !isNull(); }
  bool isNull() const { return ms == EONSAGO; }
  QDate date() const { return toQ().date(); }
  QTime time() const { return toQ().time(); }
  DateTime addYears(int n) const;
  DateTime addMonths(int n) const;
  DateTime addDays(int n) const;
  DateTime addSeconds(int n) const;
  QString const &toString() const;
  QString toString(QString fmt) const;
  QDateTime const &toQ() const;
  DateTime plusonesec() const { return addSeconds(1); }
  bool operator<(DateTime const &x) const { return ms < x.ms; }
  bool operator<=(DateTime const &x) const { return ms <= x.ms; }
  bool operator==(DateTime const &x) const { return ms == x.ms; }
  bool operator>(DateTime const &x) const { return ms > x.ms; }
  bool operator>=(DateTime const &x) const { return ms >= x.ms; }
  int toSecsSinceEpoch() const { return ms/1000; }
  quint64 toMSecsSinceEpoch() const { return ms; }
private:
  qint64 ms;
  mutable QDateTime dt;
  mutable QString str;
};

#endif
