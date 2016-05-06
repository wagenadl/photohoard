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
  void writeToDB(quint64 vsn, class PhotoDB &db) const;
  Adjustments const &baseAdjustments() const;
  Adjustments &baseAdjustments();
  int layerCount() const;
  Layer const &layer(int n) const;
  Layer &layer(int n);
  Adjustments const &layerAdjustments(int n) const;
  Adjustments &layerAdjustments(int n);
private:
  Adjustments base;
  QList<Layer> layers;
  QList<Adjustments> layadj;
};

#endif
