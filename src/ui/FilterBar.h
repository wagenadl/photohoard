// FilterBar.h

#ifndef FILTERBAR_H

#define FILTERBAR_H

#include "ActionBar.h"

class FilterBar: public ActionBar {
public:
  FilterBar(class LightTable *lighttable, class QMenu *menu,
            class MainWindow *parent);
};

#endif
