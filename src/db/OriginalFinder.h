// OriginalFinder.h

#ifndef ORIGINALFINDER_H

#define ORIGINALFINDER_H

#include <QObject>
#include "PhotoDB.h"
#include <QMap>
#include <QSize>
#include "Image16.h"
#include "Exif.h"

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
  void scaledOriginalAvailable(quint64 version, QSize osize, Image16 img);
  void exception(QString);
private slots:
  void freaderReady(QString fn);
  void rreaderReady(QString fn);
  void read(class InterruptableReader *);
  void fixOrientation(Image16 &);
private:
  PhotoDB db;
  class BasicCache *cache;
  class InterruptableFileReader *filereader;
  class InterruptableRawReader *rawreader;
private:
  quint64 version;
  QSize desired;
  QSize osize;
  QString filepath;
  Exif::Orientation orient;
};

#endif
