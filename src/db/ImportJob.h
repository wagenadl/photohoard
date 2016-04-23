// ImportJob.h

#ifndef IMPORTJOB_H

#define IMPORTJOB_H

#include <QObject>
#include <QList>
#include <QUrl>
#include "CopyIn.h"

class ImportJob: public QObject {
  Q_OBJECT;
public:
  enum class Operation {
    Import,
    Incorporate
  };
public:
  ImportJob(class SessionDB *db,
	    class Scanner *scanner,
	    QList<QUrl> const &sources,
	    QObject *parent=0);
  virtual ~ImportJob();
signals:
  void progress(int n); // for copying
  void complete(QString errmsg); // could split into photos and movies
  void canceled();
  void countsUpdated(int ntotal, int nmov);
public slots:
  void setOperation(Operation);
  void setDestination(QString);
  void setAutoDestination();
  void setMovieDestination(QString);
  void setNoMovieDestination();
  void setSourceDisposition(CopyIn::SourceDisposition);
  void setCollection(QString);
  void setAutoCollection();
  void countSources();
  void authorize();
  void cancel();
public:
  Operation operation() const { return op; }
  QString destination() const { return dest; }
  bool isAutoDestination() const { return autodest; }
  QString movieDestination() const { return moviedest; }
  CopyIn::SourceDisposition sourceDisposition() const { return srcdisp; }
  QString collection() const { return coll; }
  bool isAuthorized() const { return authorized_; }
  bool isComplete() const { return complete_; }
public:
  QList<QUrl> statedSources() const;
  bool isAnySourceNonlocal() const;
  bool hasSourceCount() const;
  int sourceCount(); // -1 if not available; will wait if needed
  int preliminarySourceCount() const;
  bool sourceIsExternalMedia() const;
  //  bool sourceIsWritableLocation() const;
  //  bool sourceIsSingleFolder() const;
  //  bool sourceInvolvesOnlyFiles() const;
public:
  class SessionDB *database() const { return db; }
  class Scanner *scanner() const { return scanner_; }
public:
  QString commonRoot() const;
  static QString commonRoot(QList<QUrl> const &);
  static bool pathIsExternalMedia(QString);
private slots:
  void markFinalSourceCount();
  void doneCopying(int, int);
private:
  void startCopy();
private:
  class SessionDB *db;
  class Scanner *scanner_;
  class CopyIn *copyin;
  class Collector *collector;
private:
  QList<QUrl> sources_;
  Operation op;
  QString dest;
  bool autodest;
  QString moviedest;
  CopyIn::SourceDisposition srcdisp;
  QString coll;
  bool authorized_;
  bool complete_;
};

#endif
