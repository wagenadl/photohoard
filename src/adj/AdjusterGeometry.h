// AdjusterGEOMETRY.h

#ifndef ADJUSTERGEOMETRY_H

#define ADJUSTERGEOMETRY_H

#include "AdjusterStage.h"

class AdjusterGeometry: public AdjusterStage {
  // We do not respect ROIs yet
public:
  AdjusterGeometry() { }
  virtual ~AdjusterGeometry() { }
  virtual QStringList fields() const override;
  virtual AdjusterTile apply(AdjusterTile const &,
			     Adjustments const &) override;
  static QPointF map(QPointF, QSize osize, Adjustments const &);
};

#endif
