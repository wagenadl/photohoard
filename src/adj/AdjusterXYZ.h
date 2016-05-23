// AdjusterXYZ.h

#ifndef ADJUSTERXYZ_H

#define ADJUSTERXYZ_H

#include "AdjusterStage.h"

class AdjusterXYZ: public AdjusterStage {
public:
  AdjusterXYZ(int maxthreads=1): AdjusterStage(maxthreads) { }
  virtual ~AdjusterXYZ() { }
  virtual QStringList fields() const override;
  virtual AdjusterTile apply(AdjusterTile const &, Adjustments const &) override;
};

#endif
