// ActionBar.h

#ifndef ACTIONBAR_H

#define ACTIONBAR_H

#include <QToolBar>
#include "Action.h"


class ActionBar: public QToolBar {
public:
  ActionBar(QWidget *parent=0);
  Actions const &actions() const;
protected:
  Actions acts;
};

#endif
