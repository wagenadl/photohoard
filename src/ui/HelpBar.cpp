// HelpBar.cpp

#include "HelpBar.h"

#include <QMetaType>
#include "PDebug.h"
#include "MainWindow.h"

HelpBar::HelpBar(MainWindow *parent):
  ActionBar(parent) {
  setObjectName("Help");

  acts << Action{Qt::CTRL + Qt::Key_H, "Help",
                    [=]() { parent->showShortcutHelp(); }};
  new PAction{acts.last(), QIcon(":icons/help.svg"), this};
}

