// Layers.h

#ifndef LAYERS_H

#define LAYERS_H

#include <QPolygon>
#include <QVariant>

class Layer {
public:
  enum class Type {
    Invalid=-1,
      Base=0,
      LinearGradient,
      Circular,
      Curve,
      Area
      };
  static QString typeName(Type);
public:
  Layer();
  bool isActive() const;
  void setActive(bool);
  Type type() const;
  QString typeName() const;
  void setType(Type);
  QByteArray const &data() const;
  void setData(QByteArray const &);
  QPolygon points() const;
  void setPoints(QPolygon const &);
  QImage mask(QSize origSize, class Adjustments const &) const;
private:
  bool active;
  Type typ;
  QByteArray dat;
};

class Layers {
public:
  Layers(quint64 vsn, class PhotoDB *db);
  int count() const;
  // Note that layers are counted 1..N, N being topmost
  Layer layer(int) const;
  quint64 layerID(int) const;
public: // Following create a transaction to modify the db
  void addLayer(Layer const &);
  void raiseLayer(int);
  void lowerLayer(int);
  void deleteLayer(int);
  void setLayer(int, Layer const &);
private:
  PhotoDB *db;
  quint64 vsn;
};

#endif