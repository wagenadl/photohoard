// AllAdjustments.h

#ifndef ALLADJUSTMENTS_H

#define ALLADJUSTMENTS_H

#include "Layers.h"
#include "Adjustments.h"

#include <QList>

class AllAdjustments {
public:
  AllAdjustments();
  static AllAdjustments fromDB(quint64 vsn, class PhotoDB &db);
  void readFromDB(quint64 vsn, class PhotoDB &db);
  void rereadFromDB(quint64 vsn, int lowest, class PhotoDB &db);
  /* Rereads layer and adjustment info starting at the given LOWEST layer.
     If LOWEST=0, this is the same as readFromDB. */
  void writeToDB(quint64 vsn, class PhotoDB &db) const;
  Adjustments const &baseAdjustments() const;
  Adjustments &baseAdjustments();
  int layerCount() const; // number of adjustment layers, not including base
  Layer const &layer(int n) const; // layers are counted 1..N !
  void setLayer(int n, Layer const &);
  /* Use n = 1..N to update an existing layer.
     Use n = N+1 to create a new layer on top. This increases the
     count and automatically initializes the Adjustments for the new
     layer to all defaults.
  */
  Adjustments const &layerAdjustments(int n) const; // 1..N
  Adjustments &layerAdjustments(int n); // 1..N
  /* It is not allowed to use this to create a new layer! */
private:
  Adjustments base;
  QVector<Layer> layers; // Take care! Entry k corresponds to layer n = k+1.
  QVector<Adjustments> layadj;
};

#endif
