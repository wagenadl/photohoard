// AdjusterEQUALIZE.h

#ifndef ADJUSTEREQUALIZE_H

#define ADJUSTEREQUALIZE_H

#include "AdjusterStage.h"

class AdjusterEqualize: public AdjusterStage {
public:
  AdjusterEqualize() { }
  virtual ~AdjusterEqualize() { }
  virtual QStringList fields() const override;
  virtual AdjusterTile apply(AdjusterTile const &, Sliders const &) override;
};

#endif
