// LayoutBar.h

#ifndef LAYOUTBAR_H

#define LAYOUTBAR_H

#include "ActionBar.h"

class LayoutBar: public ActionBar {
public:
  enum class Layout {
    FullGrid=0,
      HGrid,
      VGrid,
      HLine,
      VLine,
      FullPhoto,
      Line, // not a layout: selects HLine or VLine
      HalfGrid, // not a layout: selects HGrid or VGrid
      ToggleFullPhoto, // not a layout: selects FullPhoto or previous layout
      ToggleFullScreen, // not a layout: toggles full screen display
      ToggleOrg,
      };
public:
  LayoutBar(class LightTable *lighttable, QWidget *parent);
};

#endif
