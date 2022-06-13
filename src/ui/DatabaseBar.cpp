// DatabaseBar.cpp

#include "DatabaseBar.h"

#include <QMetaType>
#include "PDebug.h"
#include "MainWindow.h"

DatabaseBar::DatabaseBar(MainWindow *parent):
  ActionBar(parent) {
  setObjectName("Database");

  acts << Action{std::vector<unsigned int>(), "Database selection",
                    [=]() { showDatabaseMenu(parent); }};
  showdb_action = new PAction{acts.last(), QIcon(":icons/database.svg"), this};
}

void DatabaseBar::showDatabaseMenu(MainWindow *mw) {
  pDebug() << "show db menu";
}
