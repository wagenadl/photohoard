// Adjustments.h

#ifndef ADJUSTMENTS_H

#define ADJUSTMENTS_H

#include <QMap>
#include <QStringList>
#include "Exif.h"

class Adjustments {
public:
  Adjustments();
  explicit Adjustments(QString kv): Adjustments() { setAll(kv); }
  void reset();
  void setAll(QString kv);
  void setAll(QMap<QString, double> const &);
  static Adjustments fromDB(quint64 vsn, class PhotoDB &db);
  void readFromDB(quint64 vsn, class PhotoDB &db);
  void writeToDB(quint64 vsn, class PhotoDB &db) const;
  /* WRITETODB - Write all values to DB
     WRITETODB(vsn, db) writes all values into the ADJUSTMENTS table of DB,
     for version VSN.
     (In fact, only values that differ from the defaults are saved; other
     rows are deleted from the table.)
     This function does not create a TRANSACTION. The caller should do that.
  */
  QString getAll() const;
  bool set(QString k, double v);
  double get(QString k) const;
  bool operator==(Adjustments const &) const;
  bool isDefault() const;
  bool isDefault(QString k) const;
  PSize cropSize(PSize photoFileSize, Exif::Orientation orient);
public:
#define ADJUSTMENT(name, dfl) double name;
#include "AdjustmentDefs.h"
#undef ADJUSTMENT
#define ADJUSTMENT(name, dfl) static constexpr double name##Default = dfl;
#include "AdjustmentDefs.h"
#undef ADJUSTMENT
  static QStringList const &keys();
  static QMap<QString, double> const &defaults();
  static double defaultFor(QString k) { return defaults()[k]; }
};

#endif
