// DatabaseBar.h

#ifndef DATABASEBAR_H

#define DATABASEBAR_H

#include "ActionBar.h"

class DatabaseBar: public ActionBar {
public:
  DatabaseBar(class MainWindow *parent);
private:
  void showDatabaseMenu(MainWindow *);
  PAction *showdb_action;
};

#endif
