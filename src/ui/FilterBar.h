// FilterBar.h

#ifndef FILTERBAR_H

#define FILTERBAR_H

//#include "ActionBar.h"
#include <QToolBar>
#include "Action.h"

class FilterBar: public QToolBar {
public:
  FilterBar(class LightTable *lighttable, QWidget *parent);
private:
  Actions actions;
};

#endif
