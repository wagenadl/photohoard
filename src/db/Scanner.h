// Scanner.h

#ifndef SCANNER_H

#define SCANNER_H

#include "BasicThread.h"
#include <QMutex>
#include <QWaitCondition>
#include <QSet>
#include "Image16.h"
#include "SessionDB.h"
#include <QStringList>
#include <QUrl>

class Scanner: public BasicThread {
  /* SCANNER - Thread for scanning folder trees
   */
  Q_OBJECT;
public:
  Scanner(SessionDB *);
  virtual ~Scanner();
public slots:
  void addTree(QString path, QString defaultCollection="",
               QStringList excluded=QStringList());
  /* ADDTREE - Main mechanism for adding an entirely new folder tree to a db
     ADDTREE(path) adds the folder PATH and all its subfolders to the db.
     ADDTREE(path, coll) attaches the tag COLL as a "default collection" to all
     photos in the tree. (This applies to photos currently present as well as
     any that will be discovered later through RESCAN.)
     ADDTREE(path, coll, excl) excludes any subfolders with pathnames EXCL.
     Both PATH and COLL should be absolute paths.
     It is OK if PATH is already inside a tree.
   */
  void excludeTree(QString path);
  /* EXCLUDETREE - Remove a subtree from the db and make sure it stays removed
     EXCLUDETREE(path) removes the folder PATH and its subfolders from the db
     and adds PATH to the list of excluded paths so that it will not be re-added
     upon RESCAN.
     EXCLUDETREE does not delete the folders or their contents from the hard disk.
   */
  void removeTree(QString path);
  /* REMOVETREE - Remove a subtree from the db
     REMOVETREE(path) removes the folder PATH and its subfolders from the db but
     does not add it to the list of excluded paths. As a consequence, if PATH is
     a subfolder of a folder that is still in the db, RESCANALL will re-add PATH
     to the db.
  */
  void rescanAll();
  /* RESCANALL - Rescan all trees in the db
     RESCANALL() is equivalent to calling RESCAN(root) for all the roots in the
     database.
   */
  void rescan(QString root);
  /* RESCAN - Rescan a folder and its subfolders
     RESCAN(root) rescans the folder ROOT and its subfolders for new, changed,
     or removed photos.
     During the rescan process, COLLECTING, PROGRESSED, and DONE signals will be
     emitted periodically. In addition, UPDATED and UPDATEDBATCH signals will
     be emitted when new or changed photos are found.
     There is not currently a mechanism to report on disappeared photos, which
     is a bug. */
signals:
  void message(QString);
  void updated(QSet<quint64> versions);
  /* UPDATED - Emitted to report photo that has been scanned
     UPDATED(ids) is emitted to report that the photo scanning process
     has finished scanning the photo upon which versions IDS are
     based. This signal is emitted once per photo. If you connect to
     UPDATED, you should not also connect to UPDATEDBATCH. */
  void updatedBatch(QSet<quint64> versions);
  /* UPDATEDBATCH - Emitted to report photos that have been scanned
     UPDATEDBATCH(ids) is emitted periodically to report that the
     photo scanning process has finished scanning the photos upon
     which versions IDS are based. For efficiency, this is emitted
     only once in a while. (But ultimately, all scanned photos are
     included in precisely one UPDATEDBATCH signal.) If you connect to
     UPDATEDBATCH, you should not also connect to UPDATED. */
  void cacheablePreview(quint64 vsn, Image16);
  /* CACHEABLEPREVIEW - A preview thumbnail has been extracted from an image file
     CACHEABLEPREVIEW(id, img) indicates that the image file upon which version ID
     is based has been scanned and that a thumbnail image is available.
     This signal is not actually emitted by the present vsn of the code. */
  void exception(QString);
  /* EXCEPTION - Used to report an exception from the workerthread
     EXCEPTION(msg) is emitted to report an exception with the given message. */
protected:
  virtual void run() override;
private:
  QStringList allRoots();
  quint64 addFolder(PhotoDB *db, quint64 parentid, QString path, QString leaf);
  quint64 addPhoto(quint64 parentid, QString leaf);
  QSet<quint64> findFoldersToScan();
  QSet<quint64> findPhotosToScan();
  void scanFolders(QSet<quint64>); // this creates a transaction
  void scanPhotos(QSet<quint64>); // this creates a transaction
  void scanFolder(quint64 folder, QSet<QString> const &excltrees);
  void scanPhoto(quint64 photo);
  int photoQueueLength();
  int folderQueueLength();
private:
  void reportFolderProgress();
  void reportPhotoProgress();
  void reportScanDone();
  QSet<QString> excludedTrees();
  quint64 findDirOrAdd(QString path, bool secondary=false);
  QMap<QString, quint64> photosInFolder(quint64 folderid) const;
  QMap<QString, quint64> subFolders(quint64 folderid) const; // leafnames->ids
  void dropPhotos(QList<quint64> photoids);
  void dropFolders(QList<quint64> folderids); // recursive
  void dropPhotosInFolder(quint64 folderid); // recursive
private:
  SessionDB *db0; // this is the original of the caller
  SessionDB db; // this is our copy in the thread
  QMap<QString, int> exts;
  int n, N;
  int m, M;
  class CacheFiller *filler;
};

#endif
