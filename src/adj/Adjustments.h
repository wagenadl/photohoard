// Adjustments.h

#ifndef ADJUSTMENTS_H

#define ADJUSTMENTS_H

#include <QMap>
#include <QStringList>
#include "Exif.h"
#include <QDebug>

class Adjustments {
  /* ADJUSTMENTS - Storage class for collection of slider settings
     ADJUSTMENTS contains slider settings for any or all of the "adjustment"
     sliders defined in AdjustmentDefs.h. An instance of ADJUSTMENTS is not
     bound to a particular version or a particular layer.
     Methods are provided to retrieve from/store in a PhotoDB database.
   */
public:
  Adjustments();
  void reset();
  void setAll(QMap<QString, double> const &);
  static Adjustments fromDB(quint64 vsn, class PhotoDB &db,
                            class Transaction *t=0);
  static Adjustments fromDB(quint64 vsn, int layer, PhotoDB &db,
                            Transaction *t=0);
  void readFromDB(quint64 vsn, PhotoDB &db, Transaction *t=0);
  void readFromDB(quint64 vsn, int layer, PhotoDB &db, Transaction *t=0);
  void writeToDB(quint64 vsn, PhotoDB &db, Transaction *t=0) const;
  void writeToDB(quint64 vsn, int layer, PhotoDB &db, Transaction *t=0) const;
  /* WRITETODB - Write all values to DB
     WRITETODB(vsn, db) writes all values into the ADJUSTMENTS table of DB,
     for the base layer of version VSN.
     WRITETODB(vsn, layer, db) writes all values into the LAYERADJUSTMENTS
     table of DB, for the given (nonbase) layer of version VSN.
     (In fact, only values that differ from the defaults are saved; other
     rows are deleted from the table.)
     These functione do not create a TRANSACTION. The caller should do that.
  */
  void readFromDBForLayer(quint64 layer,  class PhotoDB &db,
                          Transaction *t=0);
  void writeToDBForLayer(quint64 layer, class PhotoDB &db,
                         Transaction *t=0) const;
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

QDebug &operator<<(QDebug &, Adjustments const &);

#endif
