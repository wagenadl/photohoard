// AdjusterUMASK.h

#ifndef ADJUSTERUMASK_H

#define ADJUSTERUMASK_H

#include "AdjusterStage.h"

class AdjusterUMask: public AdjusterStage {
public:
  AdjusterUMask() { }
  virtual ~AdjusterUMask() { }
  virtual QStringList fields() const override;
  virtual AdjusterTile apply(AdjusterTile const &, Sliders const &) override;
};

#endif
