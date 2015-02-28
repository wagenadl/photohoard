// AdjusterStage.h

#ifndef ADJUSTERSTAGE_H

#define ADJUSTERSTAGE_H

#include "Sliders.h"
#include "AdjusterTile.h"

class AdjusterStage {
public:
  AdjusterStage() { }
  virtual ~AdjusterStage() { }
  virtual bool isDefault(Sliders const &s) const {
    return isDefault(s, fields()); }
  virtual bool isEquivalent(Sliders const &a, Sliders const &b) const {
    return isEquivalent(a, b, fields()); }
  virtual QStringList fields() const=0;
  virtual AdjusterTile apply(AdjusterTile const &parent,
			     Sliders const &settings)=0;
  /* When calling apply(), it _must_ be the case that parent is default
     for this stage. */
  static bool isDefault(Sliders const &, QStringList fields);
  static bool isEquivalent(Sliders const &, Sliders const &,
			   QStringList fields);
};

#endif
