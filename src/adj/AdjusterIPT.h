// AdjusterIPT.h

#ifndef ADJUSTERIPT_H

#define ADJUSTERIPT_H

#include "AdjusterStage.h"

class AdjusterIPT: public AdjusterStage {
public:
  AdjusterIPT() { }
  virtual ~AdjusterIPT() { }
  virtual QStringList fields() const override;
  virtual AdjusterTile apply(AdjusterTile const &, Adjustments const &) override;
};

#endif
