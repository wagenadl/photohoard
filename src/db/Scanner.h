// Scanner.h

#ifndef SCANNER_H

#define SCANNER_H

#include "BasicThread.h"
#include <QMutex>
#include <QWaitCondition>
#include <QSet>
#include "Image16.h"
#include "PhotoDB.h"

class Scanner: public BasicThread {
  Q_OBJECT;
public:
  Scanner(PhotoDB const *);
  virtual ~Scanner();
public slots:
  void addTree(QString path);
  void removeTree(QString path);
signals:
  void collecting(int N);
  void progressed(int n, int N);
  void done();
  void updated(QSet<quint64> versions);
  void cacheablePreview(quint64 vsn, Image16);
  void exception(QString);
protected:
  virtual void run() override;
private:
  quint64 addFolder(quint64 parentid, QString path, QString leaf);
  quint64 addPhoto(quint64 parentid, QString leaf);
  QSet<quint64> findFoldersToScan();
  QSet<quint64> findPhotosToScan();
  void scanFolders(QSet<quint64>); // this creates a transaction
  void scanPhotos(QSet<quint64>); // this creates a transaction
  void scanFolder(quint64 folder);
  void scanPhoto(quint64 photo);
  int photoQueueLength();
private:
  PhotoDB db;  
  QMap<QString, int> exts;
  int n, N;
  class CacheFiller *filler;
};

#endif
