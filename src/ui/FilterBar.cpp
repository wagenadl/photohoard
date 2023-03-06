// FilterBar.cpp

#include "FilterBar.h"

#include <QMetaType>
#include "PDebug.h"
#include "LightTable.h"
#include "MainWindow.h"

FilterBar::FilterBar(LightTable *lighttable, MainWindow *parent):
  ActionBar(parent) {
  setObjectName("Filter");

  acts << Action{Qt::CTRL + Qt::Key_F, "Filter",
                    [=]() { lighttable->openFilterDialog(); }};
  new PAction{acts.last(), QIcon(":icons/search.svg"), this};

  acts << Action{Qt::CTRL + Qt::Key_H, "Help",
                    [=]() { parent->showShortcutHelp(); }};
  new PAction{acts.last(), QIcon(":icons/help.svg"), this};

  acts << Action{Qt::CTRL + Qt::SHIFT + Qt::Key_M, "Menu",
                    [=]() { parent->showMenu(); }};
  new PAction{acts.last(), QIcon(":icons/menu.svg"), this};
  
}
