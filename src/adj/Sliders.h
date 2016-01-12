// Sliders.h

#ifndef SLIDERS_H

#define SLIDERS_H

#include <QMap>
#include <QStringList>
#include "Exif.h"

class Sliders {
public:
  Sliders();
  explicit Sliders(QString kv): Sliders() { setAll(kv); }
  void reset();
  void setAll(QString kv);
  void setAll(QMap<QString, double> const &);
  static Sliders fromDB(quint64 vsn, class PhotoDB &db);
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
  bool operator==(Sliders const &) const;
  bool isDefault() const;
  bool isDefault(QString k) const;
  PSize cropSize(PSize photoFileSize, Exif::Orientation orient);
public:
#define SLIDER(name, dfl) double name;
#include "sliders.def"
#undef SLIDER
#define SLIDER(name, dfl) static constexpr double name##Default = dfl;
#include "sliders.def"
#undef SLIDER
  static QStringList const &keys();
  static QMap<QString, double> const &defaults();
  static double defaultFor(QString k) { return defaults()[k]; }
};

#endif
