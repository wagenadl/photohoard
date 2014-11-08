// CacheFiller.h

#ifndef CACHEFILLER_H

#define CACHEFILLER_H

#include "BasicThread.h"
#include "PhotoDB.h"
#include "BasicCache.h"

class CacheFiller: public BasicThread {
  /* This is a thread that will work in the background to cache all images
     contained in a PhotoDB.
     It can do so directly for original versions (version=0 in versions table),
     but it needs help for derived functions. That help is not yet programmed
     in.
  */
  Q_OBJECT;
public:
  CacheFiller(PhotoDB const &, BasicCache *);
  virtual ~CacheFiller();
  void recacheOne(quint64 version);
public slots:
  void recache(QSet<quint64> versions); // this locks the db while working
signals:
  void progressed(int n, int N);
  void done();
protected:
  virtual void run() override;
private:
  QSet<quint64> checkQueue();
  bool processSome(QSet<quint64>);
private:
  PhotoDB db;
  BasicCache *cache;
  int n, N;
};

#endif
