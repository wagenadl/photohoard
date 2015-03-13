// Stage.h

#ifndef STAGE_H

#define STAGE_H

enum Stage {
  Stage_Original,
  Stage_Reduced, // either by ROI or scale
  Stage_XYZ,
  Stage_IPT,
  Stage_Geometry,
};
/* Order of stages here must match Adjuster::retrieveXXX */

#endif
