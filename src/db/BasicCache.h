// BasicCache.h

#ifndef BASICCACHE_H

#define BASICCACHE_H

#include <QDir>
#include <QSet>
#include "Image16.h"
#include "Database.h"

class BasicCache: public QObject {
public:
  BasicCache(QString rootdir, QObject *parent=0);
  /*:F constructor
   *:D Opens the cache located at ROOTDIR.
   */
  virtual ~BasicCache();
  static BasicCache *create(QString rootdir);
  /*:F create
   *:D Creates a new basic cache located at ROOTDIR. The directory must not
   already exist.
  */
  Database &database() { return db; }
  void add(quint64 vsn, Image16 img, bool instantlyOutdated=false);
  /*:F add
   *:D Adds an image to the cache at each of the sizes defined in the cache.
   *:N By default, the sizes are 1024, 384, and 128 for the largest dimension.
   This is determined in the setupcache.sql code.
   If IMG is smaller than the largest size, IMG itself is stored in the
   cache.
  */
  void remove(quint64 vsn);
  /*:F remove
   *:D Removes all sizes of the referenced image from the cache.
   */
  Image16 get(quint64 vsn, PSize s, bool *outdated_return=NULL);
  /*:F get
   *:D Retrieves an image from the cache.
   *:N This only succeeds if the image exists at the given size. See also
   bestSize().
  */
  PSize bestSize(quint64 vsn, PSize desired);
  /*:F bestSize
   *:D Determines the best available size of the referenced image.
   *:N If a size greater or equal to MAXDIM is available, the smallest
   such size is returned. If no such sizes exist, the largest size
   available is returned.
   *:N Outdated versions are only returned if there is no up-to-date version
   available at all.
  */
  bool contains(quint64 vsn, bool outdatedOK=false);
  /*:F contains
   *:D Returns true if any images exist in the cache that match VSN.
   *:N By default, only non-outdated images are considered. This can be
   changed by passing true for outdatedOK.
  */
  void markOutdated(quint64 vsn);
  QList<PSize> sizes(quint64 vsn, bool outdatedOK=false);
  /*:F sizes
   *:D Returns a list of sizes available for images matching VSN, sorted
   by their maxdim.
   *:N By default, only non-outdated images are considered. This can be
   changed by passing true for outdatedOK.
  */      
  QList<PSize> standardSizes() const { return stdsizes; }
  PSize maxSize() const; // max of all standard sizes
private:
  BasicCache(QDir root, Database const &db, QObject *parent=0 );
  void addToCache(quint64 vsn, Image16 const &img,
		  bool instantlyOutdated=false);
  void dropOutdatedFromCache(quint64 vsn);
  void readConfig();
  QString constructFilename(quint64 vsn, int d);
  void attach();
private:
  QDir root;
  Database db;
  QList<PSize> stdsizes; // in decreasing order
  int memthresh;
};

#endif
