// Layers.h

#ifndef LAYERS_H

#define LAYERS_H

#include <QPolygon>
#include <QVariant>

class Layer {
public:
  enum class Type {
    Invalid=-1, // no data
    Base=0, // no data
    LinearGradient=1, // data is two points

    Curve=3, // data is N points and one radius (blur)
    Area=4, // data is N points and one radius (blur)
    Clone=5, // data is 2N points (N sources followed by N target) and N radii
    Inpaint=6 // data is N points and N radii
  };
  static QString typeName(Type);
public:
  Layer();
  static Layer fromDB(quint64 vsn, int stacking, class PhotoDB const &db);
  void readFromDB(quint64 vsn, int stacking, class PhotoDB const &db);
  static Layer fromDB(quint64 layerid, class PhotoDB const &db);
  void readFromDB(quint64 layerid, class PhotoDB const &db);
  void writeToDB(quint64 vsn, int stacking, class PhotoDB &db) const;
  bool isActive() const;
  void setActive(bool);
  Type type() const;
  QString typeName() const;
  void setType(Type);
  QByteArray const &data() const;
  void setData(QByteArray const &);
  QPolygon points() const;
  QList<int> radii() const;
  void setPointsAndRadii(QPolygon const &, QList<int> const &);
  double scale(QSize origSize, class Adjustments const &baseadj,
               QSize scaledCroppedSize) const;
  QImage mask(QSize origSize, class Adjustments const &baseadj,
	      QSize scaledCroppedSize) const;
  /* MASK - Return alpha mask for layer
     Result is of size scaledCroppedSize.
     Only the "LinearGradient" and "Area" masks are implemented.
     The mask is all black if the layer is not active.
   */
  double alpha() const;
  void setAlpha(double a);
  QString name() const;
  void setName(QString);
  bool operator==(Layer const &) const;
private:
  void readFromDB(class QSqlQuery &);
private:
  bool active;
  Type typ;
  double alph;
  QString name_;
  QByteArray dat;
};

class Layers {
  /* This class is a thin handle onto db info. It does not automatically
     load information from all layers; this is done on request.
     Caution: As a consequence, functions like count() and layer(n) have
     to touch the db every time they are called.
   */
public:
  Layers(quint64 vsn, class PhotoDB *db);
  int count() const;
  // Note that layers are counted 1..N, N being topmost. The layer number n
  // here corresponds to the "stacking" column in the database.
  Layer layer(int n) const;
  quint64 layerID(int n) const;
public: // Following create a transaction to modify the db
  void addLayer(Layer const &); // to top of stack
  void raiseLayer(int n);
  void lowerLayer(int n);
  void deleteLayer(int n);
  void setLayer(int n, Layer const &);
private:
  PhotoDB *db;
  quint64 vsn;
};

#endif
