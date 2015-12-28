// ColorLabelBar.h

#ifndef COLORLABELBAR_H

#define COLORLABELBAR_H

#include "ActionBar.h"

class ColorLabelBar: public ActionBar {
public:
  ColorLabelBar(class PhotoDB *db, class LightTable *lighttable,
                QWidget *parent);
};

#endif
