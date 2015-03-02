// OriginalFinder.h

#ifndef ORIGINALFINDER_H

#define ORIGINALFINDER_H

#include <QObject>
#include "PhotoDB.h"
#include <QMap>
#include <QSize>
#include "Image16.h"

class OriginalFinder: public QObject {
  Q_OBJECT;
public:
  OriginalFinder(PhotoDB const &db, class BasicCache *cache, QObject *parent=0);
  virtual ~OriginalFinder();
public slots:
  void requestOriginal(quint64 version);
  void requestScaledOriginal(quint64 version, QSize desired);
signals:
  void originalAvailable(quint64 version, Image16 img);
  void scaledOriginalAvailable(quint64 version, QSize requested, Image16 img);
  void exception(QString);
private:
  void readFTypes();
private:
  PhotoDB db;
  class BasicCache *cache;
  QMap<quint64, QString> folders;
  QMap<int, QString> ftypes;
  class IF_Worker *ifinder;
};

#endif
