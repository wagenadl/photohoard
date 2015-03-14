// AdjusterUmask.cpp

#include "AdjusterUMask.h"
#include "Adjuster.h"

QStringList AdjusterUMask::fields() const {
  static QStringList flds
    = QString("umask umaskr").
    split(" ");
  return flds;
}

AdjusterTile AdjusterUMask::apply(AdjusterTile const &parent,
				Sliders const &final) {
  AdjusterTile tile = parent;
  
  tile.stage = Stage_UMask;
  tile.image.convertTo(Image16::Format::IPT16);

  // Do unsharp mask only on I channel?
  // Alternatively, I could work in LMS space and treat all channels
  
  tile.settings.umask = final.umask;
  tile.settings.umaskr = final.umaskr;

  return tile;
}
