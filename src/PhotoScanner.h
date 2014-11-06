// PhotoScanner.h

#ifndef PHOTOSCANNER_H

#define PHOTOSCANNER_H

#include <QThread>
#include <QSqlDatabase>

class PhotoScanner: public QThread {
public:
  PhotoScanner(QSqlDatabase const &db);
  void start();
  void stop();
  void rescan(quint64 photo);
  void add(quint64 folder, QString leafname);
protected:
private:
  QSqlDatabase db;
};

#endif
