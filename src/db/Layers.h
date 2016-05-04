// Layers.h

#ifndef LAYERS_H

#define LAYERS_H

#include <QPolygon>
#include <QVariant>

struct Layer {
public:
  enum class Type {
    Invalid,
    LinearGradient,
      Circular,
      Curve,
      Area
      };
public:
  quint64 id;
  bool active;
  Type type;
  QByteArray data;
public:
  Layer();
  QPolygon points() const;
  void setPoints(QPolygon const &);
  QImage mask(QSize origSize, class Adjustments const &) const;
};

class Layers {
public:
  Layers(quint64 vsn, class PhotoDB *db);
  int size() const;
  // Note that layers are counted 1..N, N being topmost
  Layer const &layer(int) const;
public: // Following create a transaction to modify the db
  void addLayer(Layer const &, bool notrans=false);
  void raiseLayer(int, bool notrans=false);
  void lowerLayer(int, bool notrans=false);
  void deleteLayer(int, bool notrans=false);
  void setLayer(int, Layer const &, bool notrans=false);
private:
  PhotoDB *db;
  quint64 vsn;
  QList<Layer> layers;
};

#endif
