// CopyIn.h

#ifndef COPYIN_H

#define COPYIN_H

#include <QThread>
#include <QList>
#include <QUrl>

class CopyIn: public QThread {
  Q_OBJECT;
public:
  enum SourceDisposition {
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
public:
  static QString autoDest(QString path="");
  /* AUTODEST - Calculate automatic destination based on date
     AUTODEST() returns a string of the form "YYYY/YYMMDD", to be used
     as an automatic default destination for today.
     AUTODEST(path) appends that string to a specified PATH.
  */
public slots:
  void setSources(QList<QUrl>);
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
  void setMovieDestination(QString);
  /* SETMOVIEDESTINATION - Set destination for movie files
     Photohoard does not presently catalog movie files, but external media
     often do contain movies. In that case, SETMOVIEDESTINATION enables
     copying those movie files to some location on the hard disk.
   */
  void setNoMovieDestination();
  /* SETNOMOVIEDESTINATION - Specify that movies are to be left in place
     Even if the source disposition is Backup or Delete, movies will be
     left untouched in their original locations.
  */
  void start();
  /* START - Start the process
     You must call ISVALID() first. Otherwise, the program will terminate.
  */
  void cancel();
  /* CANCEL - Cancel the process
     Note: For right now, we don't provide a GUI to call cancel.
     No effect if not running.
   */
signals:
  void canceled();
  /* CANCELED - Emitted after CANCEL is called. */
  void completed(int nOk, int nFail);
  /* COMPLETED - Emitted when the job finishes
     COMPLETED(nOk, nFail) is emitted when the job is complete, even if
     some errors occurred, but not if the job was canceled by the user,
     in which case CANCELED is emitted instead.
  */
private:
  virtual void run() override;
private:
  QList<QUrl> src;
  QString dest;
  QString moviedest;
  SourceDisposition srcdisp;
private:
  bool cancel_;
};

#endif
