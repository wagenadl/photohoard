// FolderScanner.h

#ifndef FOLDERSCANNER_H

#define FOLDERSCANNER_H

#include <QThread>
#include <QSqlDatabase>
#include "PhotoScanner.h"

class FolderScanner: public QThread {
public:
FolderScanner(QSqlDatabase const &db, PhotoScanner *phscan);
  void start();
  void stop();
  void rescan(quint64 folder);
  void add(QString dirpath);
protected:
  
private:
  QSqlDatabase db;
  QPointer<PhotoScanner> phscan;
};

#endif
