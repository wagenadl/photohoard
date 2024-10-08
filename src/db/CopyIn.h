// CopyIn.h

#ifndef COPYIN_H

#define COPYIN_H

#include <QThread>
#include <QStringList>

class CopyIn: public QThread {
  Q_OBJECT;
public:
  enum class SourceDisposition {
    Leave,
    Backup,
    Delete,
  };
public:
  CopyIn(QObject *parent=0);
  virtual ~CopyIn();
  bool isValid() const;
  /* ISVALID - Determine whether we are good to go
     Call ISVALID() to determine whether we are ready for START().
     That is, a destination must have been set, and valid sources must
     have been specified.
     That is, all sources must be local and readable, and the destination
     must be writable or inside a writable tree.
  */
public slots:
  void setSources(QStringList);
  /* SETSOURCES - Specify what is going to be copied
     SETSOURCES(urls) must be called _before_ calling START().
  */
  void setDestination(QString);
  /* SETDESTINATION - Specify where things will be copied to
     SETDESTINATION(dst) specifies DST as the folder where things will be
     stored. DST must be either an existing writable directory or a path
     inside a tree that can be written.
  */
  void setSourceDisposition(SourceDisposition);
  /* SETSOURCEDISPOSITION - Specify what will be done with the sources at end
     When copying is complete, sources can be left in place, moved to a local
     backup location, or deleted entirely. SETSOURCEDISPOSITION(action),
     where action is one of Leave, Backup, or Delete, determines what happens.
     Note: If there are any errors, this action is not executed.
  */
  void setBackupLocation(QString);
  /* SETBACKUPLOCATION - Set backup location for sources
     Only used when SOURCEDISPOSITION is BACKUP, this determines where the
     source files will be moved to.
  */
  void setMovieSources(QStringList);
  void setMovieDestination(QString);
  /* SETMOVIEDESTINATION - Set destination for movie files
     Photohoard does not presently catalog movie files, but external media
     often do contain movies. In that case, SETMOVIEDESTINATION enables
     copying those movie files to some location on the hard disk.
   */
  void start();
  /* START - Start the process
     You must call ISVALID() first. Otherwise, the program will terminate.
  */
  void cancel();
  /* CANCEL - Cancel the process
     Deletes all files copied so far.
     No effect if not running.
   */
signals:
  void canceled();
  /* CANCELED - Emitted after CANCEL is called. */
  void progress(int n);
  void completed(int nOk, int nFail);
  /* COMPLETED - Emitted when the job finishes
     COMPLETED(nOk, nFail) is emitted when the job is complete, even if
     some errors occurred, but not if the job was canceled by the user,
     in which case CANCELED is emitted instead.
  */
private:
  virtual void run() override;
  void disposeSources(QStringList ss);
  void backupSources(QStringList ss);
  void deleteSources(QStringList ss);
private:
  QStringList imgSources;
  QStringList movSources;
  QString dest;
  QString moviedest;
  SourceDisposition srcdisp;
  QString bkloc;
private:
  bool cancel_;
};

#endif
