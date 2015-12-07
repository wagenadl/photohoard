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
  enum class AcceptReject {None=0, Accept=1, Reject=-1 };
public:
  struct VersionRecord {
    quint64 id;
    quint64 photo;
    int starrating;
    ColorLabel colorlabel;
    AcceptReject acceptreject;
    Exif::Orientation orient;
  };
  struct PhotoRecord {
    quint64 id;
    quint64 folderid;
    QString filename;
    int filetype;
    PSize filesize; // not taking orientation into account!
    int cameraid;
    int lensid;
    double exposetime_s;
    double fnumber;
    double focallength_mm;
    double distance_m;
    double iso;
    QDateTime capturedate;
  };
public:
  PhotoDB(QString id=""): Database(id) { }
  virtual void open(QString fn) override;
  virtual void clone(PhotoDB const &);
  static void create(QString fn);
public: // information about photos and versions
  quint64 photoFromVersion(quint64 versionid) const;
  QDateTime captureDate(quint64 photoid) const;
  QString ftype(int filetypeid) const;
  PhotoRecord photoRecord(quint64 photoid) const;
  PSize photoSize(quint64 photoid) const; // size from file, not orientation corrected
  VersionRecord versionRecord(quint64 versionid) const;
  QString make(int cameraid) const;
  QString camera(int cameraid) const; // i.e., make and model
  QString cameraAlias(int cameraid) const;
  QString model(int cameraid) const;
  QString lens(int lensid) const;
  QString lensAlias(int lensid) const;
  void setColorLabel(quint64 versionid, ColorLabel label);
  void setStarRating(quint64 versionid, int stars);
  void setAcceptReject(quint64 versionid, AcceptReject label);
  void addUndoStep(quint64 versionid, QString key,
                   QVariant oldvalue, QVariant newvalue);
public: // information about cameras and lenses
  void setLensAlias(int lensid, QString alias);
  void setCameraAlias(int cameraid, QString alias);
public: // exploration functions
  QString folder(quint64 folderid) const; // returns full pathname
  quint64 root(quint64 folderid) const; // returns id of root folder
  quint64 findFolder(QString) const; // 0 if not found
  int countInDateRange(QDateTime t0, QDateTime t1) const; // t1 is exclusive
  QList<quint64> versionsInDateRange(QDateTime t0, QDateTime t1) const;
  QDateTime firstDateInRange(QDateTime t0, QDateTime t1) const;
  QDateTime lastDateInRange(QDateTime t0, QDateTime t1) const;
  int countInFolder(QString folder) const;
  int countInTree(QString rootfolder) const;
  bool anyInTreeBelow(QString rootfolder) const;
  // _not_ counting this folder itself
  QList<quint64> versionsInFolder(QString folder) const;
  QList<QString> subFolders(QString folders) const;
  QList<QString> rootFolders() const;
  quint64 firstVersionInTree(QString folder) const;
  // all of the above look at the filter
private:
  mutable QMap<quint64, QString> folders;
  mutable QMap<QString, quint64> revFolders;
  mutable QMap<int, QString> ftypes;
  mutable QMap<int, QString> makes, models, lenses, cameraAliases, lensAliases;
};

#endif
