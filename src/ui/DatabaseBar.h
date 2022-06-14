// DatabaseBar.h

#ifndef DATABASEBAR_H

#define DATABASEBAR_H

#include "ActionBar.h"

class DatabaseBar: public ActionBar {
public:
  DatabaseBar(class SessionDB *sdb, class MainWindow *parent);
private:
  void showDatabaseMenu();
  PAction *showdb_action;
  class DatabaseDialog *dlg;
};

#endif
