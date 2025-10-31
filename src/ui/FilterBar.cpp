// FilterBar.cpp

#include "FilterBar.h"

#include <QMenu>
#include <QMetaType>
#include "PDebug.h"
#include "LightTable.h"
#include "MainWindow.h"
#include <QCursor>

FilterBar::FilterBar(LightTable *lighttable, QMenu *menu, MainWindow *parent):
  ActionBar(parent) {
  setObjectName("Filter");

  acts << Action{Qt::CTRL | Qt::Key_F, "Filter",
                    [=]() { lighttable->openFilterDialog(); }};
  new PAction{acts.last(), QIcon(":icons/search.svg"), this};

//  acts << Action{Qt::CTRL | Qt::Key_H, "Help",
//                    [=]() { parent->showShortcutHelp(); }};
//  new PAction{acts.last(), QIcon(":icons/help.svg"), this};

  acts << Action{Qt::CTRL | Qt::SHIFT | Qt::Key_M, "Menu",
      [menu]() {
        menu->move(QCursor::pos() - QPoint(100, -10));
        menu->show(); }};
  PAction *amenu = new PAction{acts.last(), QIcon(":icons/menu.svg"), this};
  //amenu->setMenu(menu);
  
}
