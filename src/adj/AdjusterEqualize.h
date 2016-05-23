// AdjusterEQUALIZE.h

#ifndef ADJUSTEREQUALIZE_H

#define ADJUSTEREQUALIZE_H

#include "AdjusterStage.h"

class AdjusterEqualize: public AdjusterStage {
public:
  AdjusterEqualize(int maxthreads=1): AdjusterStage(maxthreads) { }
  virtual ~AdjusterEqualize() { }
  virtual QStringList fields() const override;
  virtual AdjusterTile apply(AdjusterTile const &, Adjustments const &) override;
};

#endif
