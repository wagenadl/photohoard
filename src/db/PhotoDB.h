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
  enum class AcceptReject { Undecided=0, Accept=1, Reject=-1, NewImport=2 };
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
  PhotoDB(QString id="");
  virtual void clone(PhotoDB const &);
  static void create(QString fn);
  bool isReadOnly() const;
public: // information about photos and versions
  quint64 photoFromVersion(quint64 versionid) const;
  QDateTime captureDate(quint64 photoid) const;
  QString ftype(int filetypeid) const;
  PhotoRecord photoRecord(quint64 photoid) const;
  PSize photoSize(quint64 photoid) const; // size from file, not orientation corrected
  VersionRecord versionRecord(quint64 versionid) const;
  PSize originalSize(quint64 vsnid) const;
  static PSize originalSize(VersionRecord const &vr, PhotoRecord const &pr);
  /* ORIGINALSIZE - Size of version, before cropping
     ORIGINALSIZE(vsnid) looks up the given version in the database
     and returns the size of the image before cropping.
     ORIGINALSIZE(vrec, prec) uses preloaded version and photo records.
     This does not need database lookup. */
  void setColorLabel(quint64 versionid, ColorLabel label);
  void setStarRating(quint64 versionid, int stars);
  void setAcceptReject(quint64 versionid, AcceptReject label);
  AcceptReject acceptReject(quint64 versionid) const;
  void addUndoStep(quint64 versionid, int layer, QString key,
                   QVariant oldvalue, QVariant newvalue);
  void addUndoStep(quint64 versionid, QString key,
                   QVariant oldvalue, QVariant newvalue);
public: // information about cameras and lenses
  QString make(int cameraid) const;
  QString camera(int cameraid) const; // i.e., make and model
  QString cameraAlias(int cameraid) const;
  QString model(int cameraid) const;
  QString lens(int lensid) const;
  QString lensAlias(int lensid) const;
  void setLensAlias(int lensid, QString alias);
  void setCameraAlias(int cameraid, QString alias);
public: // exploration functions
  QString cacheFilename() const; // returns full pathname
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
public: // manipulating the database
  quint64 newVersion(quint64 versionid, bool clone=true);
  /* NEWVERSION - Create a new version of an existing photo
     v1 = NEWVERSION(vsn) creates an exact clone of the version VSN.
     v1 = NEWVERSION(vsn, false) creates a new version of the photo that VSN
     refers to, but with default rather than copied sliders.
     This function does *not* notify the cache or the GUI.
   */
  void deleteVersion(quint64 versionid);
  /* DELETEVERSION - Delete a version from the database
     The associated photo is not removed, even if it is orphaned.
  */
  bool hasSiblings(quint64 versionid);
  /* HASSIBLINGS - Return true if multiple versions are associated with a photo
     HASSIBLINGS(versionid) returns true if VERSIONID is associated with
     a photo that has other versions associated with it as well.
  */
  QSet<quint64> versionsForPhoto(quint64 photoid);
  /* VERSIONSFORPHOTO - The set of versions associated with a given photo
     VERSIONSFORPHOTO(photoid) returns the set of versions associated with
     the photo PHOTOID.
  */
  void deletePhoto(quint64 photoid);
  /* DELETEPHOTO - Delete a photo from the database
     All associated versions are deleted too.
  */
protected:
  void setReadOnly();
private:
  void readFTypes() const;
private:
  bool ro;
  mutable QMap<quint64, QString> folders;
  mutable QMap<QString, quint64> revFolders;
  mutable QMap<int, QString> ftypes;
  mutable QMap<int, QString> makes, models, lenses, cameraAliases, lensAliases;
};

#endif
