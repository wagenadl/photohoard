// DatabaseBar.cpp

#include "DatabaseBar.h"

#include <QMetaType>
#include "PDebug.h"
#include "MainWindow.h"
#include "DatabaseDialog.h"

DatabaseBar::DatabaseBar(SessionDB *sdb, MainWindow *parent):
  ActionBar(parent) {
  setObjectName("Database");
  dlg = new DatabaseDialog(sdb);
  acts << Action{Qt::CTRL + Qt::SHIFT + Qt::Key_B, "Database selection",
      [=]() { showDatabaseMenu(); }};
  showdb_action = new PAction{acts.last(), QIcon(":icons/database.svg"), this};
}

void DatabaseBar::showDatabaseMenu() {
  dlg->setup();
  dlg->show();
}