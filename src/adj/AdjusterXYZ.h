// AdjusterXYZ.h

#ifndef ADJUSTERXYZ_H

#define ADJUSTERXYZ_H

#include "AdjusterStage.h"

class AdjusterXYZ: public AdjusterStage {
public:
  AdjusterXYZ() { }
  virtual ~AdjusterXYZ() { }
  virtual QStringList fields() const override;
  virtual AdjusterTile apply(AdjusterTile const &, Sliders const &) override;
};

#endif