// FilterBar.h

#ifndef FILTERBAR_H

#define FILTERBAR_H

//#include "ActionBar.h"
#include <QToolBar>
#include "Action.h"

class FilterBar: public QToolBar {
  Q_OBJECT;
public:
  FilterBar(class LightTable *lighttable, QWidget *parent);
  virtual ~FilterBar();
private:
  Actions actions;
  class LightTable *lighttable;
};

#endif
