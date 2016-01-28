// AdjusterStage.h

#ifndef ADJUSTERSTAGE_H

#define ADJUSTERSTAGE_H

#include "Adjustments.h"
#include "AdjusterTile.h"

class AdjusterStage {
public:
  AdjusterStage() { }
  virtual ~AdjusterStage() { }
  virtual bool isDefault(Adjustments const &s) const {
    return isDefault(s, fields()); }
  virtual bool isEquivalent(Adjustments const &a, Adjustments const &b) const {
    return isEquivalent(a, b, fields()); }
  virtual QStringList fields() const=0;
  virtual AdjusterTile apply(AdjusterTile const &parent,
			     Adjustments const &settings)=0;
  /* When calling apply(), it _must_ be the case that parent is default
     for this stage. */
  static bool isDefault(Adjustments const &, QStringList fields);
  static bool isEquivalent(Adjustments const &, Adjustments const &,
			   QStringList fields);
};

#endif
