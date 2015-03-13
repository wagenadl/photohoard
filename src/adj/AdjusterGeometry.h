// AdjusterGEOMETRY.h

#ifndef ADJUSTERGEOMETRY_H

#define ADJUSTERGEOMETRY_H

#include "AdjusterStage.h"

class AdjusterGeometry: public AdjusterStage {
public:
  AdjusterGeometry() { }
  virtual ~AdjusterGeometry() { }
  virtual QStringList fields() const override;
  virtual AdjusterTile apply(AdjusterTile const &, Sliders const &) override;
};

#endif
