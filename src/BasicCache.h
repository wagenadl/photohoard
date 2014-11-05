// BasicCache.h

#ifndef BASICCACHE_H

#define BASICCACHE_H

#include <QDir>
#include <QSet>
#include <QImage>
#include <QSqlDatabase>

class BasicCache {
public:
  BasicCache(QString rootdir);
  /*:F constructor
   *:D Opens the cache located at ROOTDIR.
   *:N Use the ok() function to check that opening was successful.
   */
  ~BasicCache();
  bool ok() const;
  /*:F ok
   *:D Reports whether the cache was successfully opened.
   */
  static BasicCache *create(QString rootdir);
  /*:F create
   *:D Creates a new basic cache located at ROOTDIR. The directory must not
       already exist.
  */
  void add(quint64 id, QImage img);
  /*:F add
   *:D Adds an image to the cache at each of the sizes defined in the cache.
   *:N By default, the sizes are 1024, 384, and 128 for the largest dimension.
       This is determined in the setupcache.sql code.
       If IMG is smaller than the largest size, IMG itself is stored in the
       cache.
   */
  void remove(quint64 id);
  /*:F remove
   *:D Removes all sizes of the referenced image from the cache.
   */
  QImage get(quint64 id, int maxdim, bool *outdated_return=NULL);
  /*:F get
   *:D Retrieves an image from the cache.
   *:N This only succeeds if the image exists at the given size. See also
       bestSize().
  */
  int bestSize(quint64 id, int maxdim);
  /*:F bestSize
   *:D Determines the best available size of the referenced image.
   *:N If a size greater or equal to MAXDIM is available, the smallest
       such size is returned. If no such sizes exist, the largest size
       available is returned.
   *:N Outdated versions are only returned if there is no up-to-date version
       available at all.
  */
  bool contains(quint64 id, bool outdatedOK=false);
  /*:F contains
   *:D Returns true if any images exist in the cache that match ID.
   *:N By default, only non-outdated images are considered. This can be
       changed by passing true for outdatedOK.
  */
  QList<QSize> sizes(quint64 id, bool outdatedOK=false);
  /*:F sizes
   *:D Returns a list of sizes available for images matching ID, sorted
       by their maxdim.
   *:N By default, only non-outdated images are considered. This can be
       changed by passing true for outdatedOK.
  */      
  static int maxdim(QSize const &s);
  /*:F maxdim
   *:D Convenience function to return the larger of the two dimensions
       contained in a QSize.
  */
private:
  BasicCache(QDir root, QSqlDatabase const &db);
  void addToCache(quint64 id, QImage const &img);
  void readConfig();
private:
  QDir root;
  QSqlDatabase db;
  QList<int> stdsizes; // in decreasing order
  int memthresh;
private:
  BasicCache(BasicCache const &) = delete;
  BasicCache &operator=(BasicCache const &) = delete;
};

#endif
