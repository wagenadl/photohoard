// ColorLabelBar.h

#ifndef COLORLABELBAR_H

#define COLORLABELBAR_H

#include <QToolBar>
#include "Action.h"

class ColorLabelBar: public QToolBar {
public:
  ColorLabelBar(class PhotoDB *db, class LightTable *lighttable,
                QWidget *parent);
private:
  Actions actions;
};

#endif
