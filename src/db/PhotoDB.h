// PhotoDB.h

#ifndef PHOTODB_H

#define PHOTODB_H

#include "Database.h"
#include <QDateTime>
#include <QSharedPointer>
#include <QMap>
#include "Exif.h"
#include <QSet>

class PhotoDB: public Database {
public:
  enum class ColorLabel { None=0, Red, Yellow, Green, Blue, Purple };
  enum class AcceptReject {None=0, Accept=1, Reject=-1 }
public:
  struct VersionRecord {
    quint64 id;
    quint64 photo;
    QString mods;
    int starrating;
    ColorLabel colorlabel;
    AcceptReject acceptreject;
  };
  struct PhotoRecord {
    quint64 id;
    quint64 folderid;
    QString filename;
    int filetype;
    PSize photosize; // not taking orientation into account!
    int cameraid;
    int lensid;
    double exposetime_s;
    double fnumber;
    double focallength_mm;
    double distance_m;
    double iso;
    Exif::Orientation orient;
    QDateTime capturedate;
  };
  struct PhotoSize {
    PSize filesize;
    Exif::Orientation orient;
  };
public:
  PhotoDB(QString fn);
  static PhotoDB create(QString fn);
  quint64 photoFromVersion(quint64 versionid);
  QDateTime captureDate(quint64 photoid);
  QString ftype(int filetypeid) const;
  QString folder(quint64 folderid); // returns full pathname
  quint64 root(quint64 folderid); // returns id of root folder
  PhotoRecord &&photoRecord(quint64 photoid);
  PhotoSize &&photoSize(quint64 photoid);
  VersionRecord &&versionRecord(quint64 versionid);
  QString camera(int cameraid);
  QString lens(int lensid);
  int findTag(QString tag); // 0 for not found
  QString tag(int tagid);
  QSet<int> descendantTags(int tagid);
  QSet<int> childTags(int tagid);
  QSet<int> appliedTags(quint64 versionid);
  void setColorLabel(quint64 versionid, ColorLabel label);
  void setStarRating(quint64 versionid, int stars);
  void setAcceptReject(quint64 versionid, AcceptReject label);
  void addTag(quint64 versionid, int tagid);
  void removeTag(quint64 versionid, int tagid);
  int defineTag(QString tag, int parent);
private:
  QSharedPointer< QMap<quint64, QString> > folders;
  QSharedPointer< QMap<int, QString> > ftypes;
  QSharedPointer< QMap<int, QString> > cameras;
  QSharedPointer< QMap<int, QString> > lenses;
};

#endif
