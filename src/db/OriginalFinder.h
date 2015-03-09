// OriginalFinder.h

#ifndef ORIGINALFINDER_H

#define ORIGINALFINDER_H

#include <QObject>
#include "PhotoDB.h"
#include <QMap>
#include "PSize.h"
#include "Image16.h"
#include "Exif.h"

class OriginalFinder: public QObject {
  Q_OBJECT;
public:
  OriginalFinder(PhotoDB const &db, QObject *parent=0);
  virtual ~OriginalFinder();
public slots:
  void requestOriginal(quint64 version);
  void requestScaledOriginal(quint64 version, QSize desired);
signals:
  void originalAvailable(quint64 version, Image16 img);
  void scaledOriginalAvailable(quint64 version, QSize osize, Image16 img);
  void exception(QString);
private slots:
  void provide(QString fn, QByteArray);
private:
  void fixOrientation(Image16 &);
private:
  PhotoDB db;
  class InterruptableFileReader *filereader;
  class InterruptableRawReader *rawreader;
private:
  quint64 version;
  PSize desired;
  PSize osize;
  QString filepath;
  Exif::Orientation orient;
};

#endif
