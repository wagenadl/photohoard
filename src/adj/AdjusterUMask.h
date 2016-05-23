// AdjusterUMASK.h

#ifndef ADJUSTERUMASK_H

#define ADJUSTERUMASK_H

#include "AdjusterStage.h"

class AdjusterUMask: public AdjusterStage {
public:
  AdjusterUMask(int maxthreads=1): AdjusterStage(maxthreads) { }
  virtual ~AdjusterUMask() { }
  virtual QStringList fields() const override;
  virtual AdjusterTile apply(AdjusterTile const &, Adjustments const &) override;
};

#endif
